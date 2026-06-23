#include "AudioAnalyzer.h"
#include <cmath>
#include <numeric>
#include <algorithm>

// ─── Simple Cooley–Tukey FFT (in-place, power-of-two) ───────────────────────
static void fft(std::vector<float>& re, std::vector<float>& im) {
    int n = (int)re.size();
    // Bit-reversal permutation
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) { std::swap(re[i], re[j]); std::swap(im[i], im[j]); }
    }
    // FFT butterfly
    for (int len = 2; len <= n; len <<= 1) {
        float angle = -2.f * float(M_PI) / len;
        float wRe = std::cos(angle), wIm = std::sin(angle);
        for (int i = 0; i < n; i += len) {
            float curRe = 1.f, curIm = 0.f;
            for (int j = 0; j < len / 2; ++j) {
                float uRe = re[i+j], uIm = im[i+j];
                float vRe = re[i+j+len/2]*curRe - im[i+j+len/2]*curIm;
                float vIm = re[i+j+len/2]*curIm + im[i+j+len/2]*curRe;
                re[i+j]          = uRe + vRe;  im[i+j]          = uIm + vIm;
                re[i+j+len/2]    = uRe - vRe;  im[i+j+len/2]    = uIm - vIm;
                float tmpRe = curRe*wRe - curIm*wIm;
                curIm = curRe*wIm + curIm*wRe; curRe = tmpRe;
            }
        }
    }
}

// ────────────────────────────────────────────────────────────────────────────

void AudioAnalyzer::setup(int sr, int fSize) {
    sampleRate = sr;
    fftSize    = fSize;

    // Build Hann window
    window.resize(fftSize);
    for (int i = 0; i < fftSize; ++i)
        window[i] = 0.5f * (1.f - std::cos(2.f * float(M_PI) * i / (fftSize - 1)));

    fftReal .assign(fftSize, 0.f);
    fftImag .assign(fftSize, 0.f);
    magnitude.assign(fftSize / 2, 0.f);
}

// ─── Called from audio callback ─────────────────────────────────────────────
void AudioAnalyzer::process(const float* samples, int numSamples, AudioFeatures& out) {

    // 1. RMS ─────────────────────────────────────────────────────────────────
    out.rms = computeRMS(samples, numSamples);

    // 2. FFT ─────────────────────────────────────────────────────────────────
    computeFFT(samples, numSamples);

    // 3. Band energies ────────────────────────────────────────────────────────
    //    Guitar fundamentals roughly:  bass 80-300 Hz, mid 300-3kHz, treble 3k+
    int bassLow    = int(80.f   / binToHz(1));
    int bassHigh   = int(300.f  / binToHz(1));
    int midHigh    = int(3000.f / binToHz(1));
    int trebleHigh = int(12000.f/ binToHz(1));
    trebleHigh     = std::min(trebleHigh, (int)magnitude.size()-1);

    out.bass   = std::min(1.f, bandEnergy(bassLow,   bassHigh) * 8.f);
    out.mid    = std::min(1.f, bandEnergy(bassHigh+1, midHigh)  * 4.f);
    out.treble = std::min(1.f, bandEnergy(midHigh+1, trebleHigh)* 10.f);

    // 4. Spectrum snapshot ────────────────────────────────────────────────────
    out.spectrum = magnitude;   // copy; renderer will down-sample as needed

    // 5. Pitch (peak bin, then refined) ──────────────────────────────────────
    auto   it     = std::max_element(magnitude.begin() + bassLow,
                                     magnitude.begin() + midHigh);
    int    peakBin = (int)(it - magnitude.begin());
    float  peakHz  = peakBin * binToHz(1);
    out.pitch     = peakHz;
    out.noteIndex = -1;
    if (out.rms > 0.01f && peakHz > 50.f)
        hzToNote(peakHz, out.noteIndex);

    // 6. Onset ────────────────────────────────────────────────────────────────
    float energy  = out.rms * out.rms;
    float delta   = energy - prevEnergy;
    out.onsetFlag = (delta > onsetThresh && out.rms > 0.05f);
    prevEnergy    = energy * 0.8f + prevEnergy * 0.2f; // leaky integrator
}

// ─── Private helpers ─────────────────────────────────────────────────────────

void AudioAnalyzer::computeFFT(const float* samples, int n) {
    int copyLen = std::min(n, fftSize);
    for (int i = 0; i < fftSize; ++i) {
        fftReal[i] = (i < copyLen) ? samples[i] * window[i] : 0.f;
        fftImag[i] = 0.f;
    }
    fft(fftReal, fftImag);
    float norm = 2.f / fftSize;
    for (int i = 0; i < fftSize / 2; ++i)
        magnitude[i] = std::sqrt(fftReal[i]*fftReal[i] + fftImag[i]*fftImag[i]) * norm;
}

float AudioAnalyzer::binToHz(int bin) const {
    return bin * float(sampleRate) / float(fftSize);
}

float AudioAnalyzer::hzToNote(float hz, int& noteIdx) const {
    // A4 = 440 Hz → MIDI 69
    if (hz < 20.f) { noteIdx = -1; return -1; }
    float midi  = 69.f + 12.f * std::log2(hz / 440.f);
    int   imidi = int(std::round(midi));
    noteIdx     = ((imidi % 12) + 12) % 12;
    return midi;
}

float AudioAnalyzer::computeRMS(const float* s, int n) const {
    float sum = 0.f;
    for (int i = 0; i < n; ++i) sum += s[i] * s[i];
    return std::sqrt(sum / n);
}

float AudioAnalyzer::bandEnergy(int lo, int hi) const {
    hi = std::min(hi, (int)magnitude.size()-1);
    if (lo > hi) return 0.f;
    float e = 0.f;
    for (int i = lo; i <= hi; ++i) e += magnitude[i] * magnitude[i];
    return std::sqrt(e / (hi - lo + 1));
}
