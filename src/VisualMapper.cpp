#include "VisualMapper.h"
#include <algorithm>
#include <cmath>

void VisualMapper::setup() {
    // Nothing to initialise yet – kept for future texture/shader loading
}

VisualParams VisualMapper::map(const AudioFeatures& s, const AudioFeatures& raw) {
    VisualParams p;

    // ── 1. Primary colour from detected note ─────────────────────────────────
    if (s.noteIndex >= 0 && s.noteIndex < 12)
        p.primaryColor = NOTE_COLORS[s.noteIndex];
    else
        p.primaryColor = ofColor(180, 180, 180); // neutral grey when silent

    // ── 2. Bass colour: warm at high bass, cool at low bass ──────────────────
    float bf       = std::min(1.f, s.bass * sensitivityBass);
    p.bassColor    = lerpColor(ofColor(20, 30, 80), ofColor(200, 60, 20), bf);

    // ── 3. Background brightness: quiet → near-black, loud → dim glow ────────
    p.bgBrightness = std::min(0.35f, s.rms * sensitivityRMS * 0.5f);

    // ── 4. Core radius: driven by RMS ────────────────────────────────────────
    float rmsScaled  = std::min(1.f, s.rms * sensitivityRMS);
    p.coreRadius     = 80.f + rmsScaled * 180.f;

    // ── 5. Wave amplitude: bass ───────────────────────────────────────────────
    p.waveAmplitude  = bf * 120.f;

    // ── 6. Treble glow ────────────────────────────────────────────────────────
    float tf         = std::min(1.f, s.treble * sensitivityTreble);
    p.trebleGlow     = tf * 200.f;

    // ── 7. Onset burst ────────────────────────────────────────────────────────
    p.particleBurst  = raw.onsetFlag ? 1.f : 0.f;

    // ── 8. Spectrum bars (down-sampled from full FFT) ─────────────────────────
    p.bars.resize(spectrumBars, 0.f);
    if (!s.spectrum.empty()) {
        int srcSize = (int)s.spectrum.size();
        // Use log-spaced bins to emphasise guitar range (80 Hz – 5 kHz)
        for (int i = 0; i < spectrumBars; ++i) {
            float t   = float(i) / spectrumBars;
            // Map 0..1 to log scale across 0..srcSize/2
            int src   = int(std::pow(float(srcSize / 2), t));
            src       = std::min(src, srcSize - 1);
            p.bars[i] = std::min(1.f, s.spectrum[src] * 6.f);
        }
    }

    p.noteIndex = s.noteIndex;
    p.onset     = raw.onsetFlag;

    return p;
}

ofColor VisualMapper::lerpColor(ofColor a, ofColor b, float t) {
    t = std::max(0.f, std::min(1.f, t));
    return ofColor(
        a.r + (b.r - a.r) * t,
        a.g + (b.g - a.g) * t,
        a.b + (b.b - a.b) * t
    );
}
