#pragma once

#include "ofMain.h"
#include "AudioAnalyzer.h"

struct VisualParams {
    ofColor primaryColor;      // note-driven hue
    ofColor bassColor;         // warm/cold bass tint
    float   bgBrightness;      // 0..1  background darkness

    float   coreRadius;        // central pulsing circle radius
    float   waveAmplitude;     // bass wave height
    float   particleBurst;     // 0..1 onset burst intensity
    float   trebleGlow;        // treble highlight glow radius

    std::vector<float> bars;   // normalized heights for spectrum display

    int     noteIndex;         // -1 or 0..11
    bool    onset;
};

static const ofColor NOTE_COLORS[12] = {
    ofColor(220,  50,  50),  // C  – red
    ofColor(220,  90,  50),  // C# – red-orange
    ofColor(230, 140,  40),  // D  – orange
    ofColor(240, 200,  30),  // D# – yellow
    ofColor(130, 210,  50),  // E  – yellow-green
    ofColor( 50, 210,  80),  // F  – green
    ofColor( 30, 200, 160),  // F# – teal
    ofColor( 30, 150, 220),  // G  – blue
    ofColor( 60,  80, 220),  // G# – indigo
    ofColor(120,  50, 220),  // A  – violet
    ofColor(200,  50, 200),  // A# – purple
    ofColor(220,  50, 130),  // B  – pink
};

class VisualMapper {
public:
    void setup();

    VisualParams map(const AudioFeatures& smooth, const AudioFeatures& raw);

    float sensitivityRMS    = 1.2f;
    float sensitivityBass   = 1.5f;
    float sensitivityTreble = 1.8f;
    int   spectrumBars      = 128;

private:
    ofColor lerpColor(ofColor a, ofColor b, float t);
};
