#pragma once

#include "ofMain.h"
#include <vector>
#include <array>

// ─────────────────────────────────────────────
//  Plain data struct shared between threads
// ─────────────────────────────────────────────
struct AudioFeatures {
    float rms          = 0.f;   // overall loudness  [0..1]
    float bass         = 0.f;   // low-freq energy   [0..1]
    float mid          = 0.f;   // mid-freq energy   [0..1]
    float treble       = 0.f;   // high-freq energy  [0..1]
    float pitch        = 0.f;   // dominant freq in Hz
    int   noteIndex    = -1;    // 0=C, 1=C#, … 11=B  (-1=none)
    bool  onsetFlag    = false; // true for ONE frame when attack detected
    std::vector<float> spectrum; // full FFT magnitude bins (normalized)
};


class AudioAnalyzer {
public:

void setup(int sampleRate, int fftSize = 2048);

    void process(const float* samples, int numSamples, AudioFeatures& out);

private:
    // ── FFT helpers ──────────────────────────
    void computeFFT(const float* samples, int n);
    float binToHz(int bin) const;
    float hzToNote(float hz, int& noteIdx) const;

    // ── Onset / energy ───────────────────────
    float computeRMS(const float* s, int n) const;
    float bandEnergy(int binLow, int binHigh) const;

    // ── Hann window ──────────────────────────
    std::vector<float> window;

    // ── FFT buffers ──────────────────────────
    int   fftSize    = 2048;
    int   sampleRate = 44100;
    std::vector<float> fftReal;   // time-domain (windowed)
    std::vector<float> fftImag;
    std::vector<float> magnitude; // |FFT| bins [0 .. fftSize/2]

    // ── Onset detection ──────────────────────
    float prevEnergy   = 0.f;
    float onsetThresh  = 0.08f;  // tune to taste

    // ── Note names ───────────────────────────
    static constexpr std::array<const char*, 12> NOTE_NAMES = {
        "C","C#","D","D#","E","F","F#","G","G#","A","A#","B"
    };
};
