#pragma once

#include "ofMain.h"
#include "AudioAnalyzer.h"
#include "VisualMapper.h"
#include "ParticleSystem.h"

class ofApp : public ofBaseApp {
public:
    void setup() override;
    void update() override;
    void draw() override;
    void exit() override;

    void audioIn(ofSoundBuffer& buffer) override;

    void keyPressed(int key) override;
    void drawUI();

private:
    // --- Audio ---
    ofSoundStream   soundStream;
    AudioAnalyzer   analyzer;

    // --- Visuals ---
    VisualMapper    mapper;
    ParticleSystem  particles;

    // Shared audio data (audio thread writes, render thread reads)
    std::mutex      audioMutex;
    AudioFeatures   latestFeatures;   // filled by audioIn(), read by update()
    AudioFeatures   smoothFeatures;   // smoothed copy used by draw()

    // --- Settings ---
    bool showUI      = true;
    bool showDebug   = false;
    int  sampleRate  = 44100;
    int  bufferSize  = 1024;
    int  numChannels = 1;
};
