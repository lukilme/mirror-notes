#include "ofApp.h"
#include <cmath>
#include <algorithm>

static const char* NOTE_NAMES[12] = {
    "C","C#","D","D#","E","F","F#","G","G#","A","A#","B"
};

void ofApp::refreshCameraList() {
    cameraDevices = camera.listDevices();
    if (!cameraDevices.empty()) {
        selectedCameraIndex = std::clamp(selectedCameraIndex, 0, (int)cameraDevices.size() - 1);
    }
}

bool ofApp::openSelectedCamera() {
    if (camera.isInitialized()) {
        camera.close();
    }

    if (cameraDevices.empty()) {
        showCamera = false;
        return false;
    }

    const int totalDevices = (int)cameraDevices.size();
    const int preferredIndex = std::clamp(selectedCameraIndex, 0, totalDevices - 1);

    for (int offset = 0; offset < totalDevices; ++offset) {
        const int candidateIndex = (preferredIndex + offset) % totalDevices;
        const std::string candidateName = cameraDevices[candidateIndex].deviceName;

        camera.setDeviceID(candidateIndex);
        camera.setDesiredFrameRate(30);
        camera.setPixelFormat(OF_PIXELS_RGB);
        camera.setup(640, 480);

        if (!camera.isInitialized()) {
            ofLogWarning("GuitarViz") << "Camera candidate " << candidateIndex
                                     << " (" << candidateName << ") failed to initialize; trying next device.";
            continue;
        }

        selectedCameraIndex = candidateIndex;
        cameraMask.allocate(640, 480, OF_IMAGE_COLOR_ALPHA);
        cameraMaskPixels.allocate(640, 480, OF_PIXELS_RGBA);
        ofLogNotice("GuitarViz") << "Camera opened on device " << candidateIndex
                                 << " (" << candidateName << ").";
        return true;
    }

    ofLogWarning("GuitarViz") << "No camera device initialized successfully; continuing without webcam input.";
    showCamera = false;
    return false;
}

void ofApp::selectCamera(int delta) {
    if (cameraDevices.empty()) {
        refreshCameraList();
    }
    if (cameraDevices.empty()) {
        return;
    }

    selectedCameraIndex = (selectedCameraIndex + delta + (int)cameraDevices.size()) % (int)cameraDevices.size();
    if (showCamera) {
        openSelectedCamera();
    }
}

void ofApp::setup() {
    Logger::info("Inicializando Guitar Visualizer");
    Logger::debug("Modo de depuração ativado");
    Logger::warning("Nenhuma câmera detectada ainda");

    ofSetWindowTitle("Guitar Visualizer");
    ofSetFrameRate(60);
    ofBackground(0);
    ofEnableSmoothing();
    ofEnableAlphaBlending();

    analyzer.setup(sampleRate, 2048);
    mapper.setup();
    particles.setup(3000);

    refreshCameraList();
    if (!cameraDevices.empty()) {
        showCamera = openSelectedCamera();
    } else {
        showCamera = false;
        ofLogWarning("GuitarViz") << "No camera devices detected.";
    }

    ofSoundStreamSettings ss;
    ss.numInputChannels  = numChannels;
    ss.numOutputChannels = 0;
    ss.sampleRate        = sampleRate;
    ss.bufferSize        = bufferSize;
    ss.numBuffers        = 4;
    ss.setInListener(this);
    soundStream.setup(ss);

    ofLogNotice("GuitarViz") << "Audio stream opened - " << sampleRate
                              << " Hz / buffer " << bufferSize;
}

void ofApp::audioIn(ofSoundBuffer& buffer) {
    AudioFeatures feat;
    analyzer.process(buffer.getBuffer().data(),
                     (int)buffer.getNumFrames(), feat);
    std::lock_guard<std::mutex> lk(audioMutex);
    latestFeatures = feat;
}

void ofApp::update() {
    float dt = ofGetLastFrameTime();

    if (showCamera && camera.isInitialized()) {
        camera.update();
        if (camera.isFrameNew()) {
            const ofPixels& src = camera.getPixels();
            const size_t w = src.getWidth();
            const size_t h = src.getHeight();
            if (cameraMaskPixels.getWidth() != w || cameraMaskPixels.getHeight() != h) {
                cameraMaskPixels.allocate(static_cast<int>(w), static_cast<int>(h), OF_PIXELS_RGBA);
                cameraMask.allocate(static_cast<int>(w), static_cast<int>(h), OF_IMAGE_COLOR_ALPHA);
            }

            for (size_t y = 0; y < h; ++y) {
                for (size_t x = 0; x < w; ++x) {
                    ofColor c = src.getColor(static_cast<int>(x), static_cast<int>(y));
                    float dominant = std::max(float(c.r), float(c.b));
                    float diff = std::max(0.f, float(c.g) - dominant);
                    bool isGreen = c.g > 80.f && diff > 30.f;
                    ofColor out(c.r, c.g, c.b, isGreen ? 0 : 255);
                    cameraMaskPixels.setColor(static_cast<int>(x), static_cast<int>(y), out);
                }
            }

            cameraMask.setFromPixels(cameraMaskPixels);
        }
    }

    AudioFeatures raw;
    {
        std::lock_guard<std::mutex> lk(audioMutex);
        raw = latestFeatures;
    }

    float alpha = std::min(1.f, dt * 8.f);   // ~8 Hz smoothing
    smoothFeatures.rms    = ofLerp(smoothFeatures.rms,    raw.rms,    alpha);
    smoothFeatures.bass   = ofLerp(smoothFeatures.bass,   raw.bass,   alpha);
    smoothFeatures.mid    = ofLerp(smoothFeatures.mid,    raw.mid,    alpha);
    smoothFeatures.treble = ofLerp(smoothFeatures.treble, raw.treble, alpha);
    smoothFeatures.pitch  = ofLerp(smoothFeatures.pitch,  raw.pitch,  alpha * 0.5f);
    smoothFeatures.noteIndex  = raw.noteIndex;     // discrete – no lerp
    smoothFeatures.onsetFlag  = raw.onsetFlag;     // pass through
    smoothFeatures.spectrum   = raw.spectrum;      // pointer swap is fast

    particles.update(dt);
}

// ─────────────────────────────────────────────────────────────────────────────
void ofApp::draw() {
    float W = ofGetWidth(), H = ofGetHeight();
    float cx = W * 0.5f, cy = H * 0.5f;
    float dt = ofGetLastFrameTime();

    AudioFeatures raw;
    {
        std::lock_guard<std::mutex> lk(audioMutex);
        raw = latestFeatures;
    }
    VisualParams vp = mapper.map(smoothFeatures, raw);

    ofSetColor(0, 0, 0, 220);
    ofDrawRectangle(0, 0, W, H);

    if (showCamera && camera.isInitialized()) {
        ofSetColor(255, 255, 255, 120 + smoothFeatures.treble * 120.f);
        cameraMask.draw(0, 0, W, H);
    }

    {
        ofPolyline wave;
        int steps = 200;
        float t   = ofGetElapsedTimef();
        for (int i = 0; i <= steps; ++i) {
            float x   = W * float(i) / steps;
            float norm = float(i) / steps;
            float y   = H * 0.75f
                       - vp.waveAmplitude * std::sin(norm * TWO_PI * 3.f + t * 2.f)
                       - vp.waveAmplitude * 0.5f * std::sin(norm * TWO_PI * 7.f - t * 3.f);
            wave.addVertex(x, y);
        }
        ofSetColor(vp.bassColor, 180);
        ofSetLineWidth(2.f);
        wave.draw();
    }

    if (!vp.bars.empty()) {
        int   nBars  = (int)vp.bars.size();
        float barW   = W / float(nBars);
        float maxH   = H * 0.35f;
        for (int i = 0; i < nBars; ++i) {
            float bh = vp.bars[i] * maxH;
            // Hue shift along bar
            float hue = float(i) / nBars * 200.f + 160.f;
            ofColor bc; bc.setHsb(hue, 200, 255, 160);
            ofSetColor(bc);
            ofDrawRectangle(i * barW, H * 0.5f - bh, barW - 1.f, bh);
        }
    }

    {
        for (int g = 5; g >= 1; --g) {
            float r   = vp.coreRadius + g * 20.f;
            int   al  = int(40.f / g);
            ofSetColor(vp.primaryColor, al);
            ofDrawCircle(cx, cy, r);
        }
        ofSetColor(vp.primaryColor, 220);
        ofDrawCircle(cx, cy, vp.coreRadius);

        ofColor hi = vp.primaryColor;
        hi.setBrightness(255); hi.setSaturation(100);
        ofSetColor(hi, 160);
        ofDrawCircle(cx - vp.coreRadius * 0.25f, cy - vp.coreRadius * 0.25f,
                     vp.coreRadius * 0.3f);
    }

    if (smoothFeatures.treble > 0.05f) {
        ofColor sparkCol = vp.primaryColor;
        sparkCol.setBrightness(255);
        particles.sparkle(cx, H * 0.15f, sparkCol, smoothFeatures.treble);
    }

    if (vp.onset) {
        particles.burst(cx, cy, vp.primaryColor, 1.f, 80);
        // Instant flash
        ofSetColor(255, 255, 255, 60);
        ofDrawRectangle(0, 0, W, H);
    }

    particles.draw();

    if (showUI) drawUI();
}

// ─────────────────────────────────────────────────────────────────────────────
void ofApp::drawUI() {
    float W = ofGetWidth(), H = ofGetHeight();

    ofSetColor(255, 200);
    ofDrawBitmapString("Guitar Visualizer  |  F1=UI  F2=Debug  C=Camera  [ / ] = Camera  ESC=Quit",
                       10, 20);

    if (!cameraDevices.empty()) {
        ofSetColor(255, 240, 120);
        ofDrawBitmapString("Cameras:", 10, 38);
        int maxEntries = std::min<int>((int)cameraDevices.size(), 4);
        for (int i = 0; i < maxEntries; ++i) {
            std::string label = std::to_string(i) + ": " + cameraDevices[i].deviceName;
            if (i == selectedCameraIndex) {
                ofSetColor(255, 255, 100);
            } else {
                ofSetColor(200, 200, 200, 180);
            }
            ofDrawBitmapString(label, 10, 54 + i * 16);
        }
    }

    auto meter = [&](const std::string& label, float val, float y) {
        ofSetColor(80);
        ofDrawRectangle(10, y, 150, 8);
        ofSetColor(val > 0.7f ? ofColor(255,80,80) : ofColor(80,200,120));
        ofDrawRectangle(10, y, val * 150.f, 8);
        ofSetColor(255);
        ofDrawBitmapString(label, 170, y + 8);
    };
    meter("RMS   ", smoothFeatures.rms,    H - 80);
    meter("Bass  ", smoothFeatures.bass,   H - 66);
    meter("Mid   ", smoothFeatures.mid,    H - 52);
    meter("Treble", smoothFeatures.treble, H - 38);

    if (smoothFeatures.noteIndex >= 0) {
        ofSetColor(255, 255, 100);
        std::string noteName = NOTE_NAMES[smoothFeatures.noteIndex];
        std::string info     = "Note: " + noteName
                             + "  Pitch: " + ofToString(smoothFeatures.pitch, 1) + " Hz";
        ofDrawBitmapString(info, 10, H - 100);
    }

    if (showDebug) {
        ofSetColor(200);
        ofDrawBitmapString("FPS: " + ofToString(ofGetFrameRate(), 1), W - 120, 20);
        ofDrawBitmapString("Buf: " + ofToString(bufferSize), W - 120, 34);
    }
}

void ofApp::keyPressed(int key) {
    if (key == OF_KEY_F1)  showUI    = !showUI;
    if (key == OF_KEY_F2)  showDebug = !showDebug;
    if (key == OF_KEY_LEFT || key == '[') {
        selectCamera(-1);
    }
    if (key == OF_KEY_RIGHT || key == ']') {
        selectCamera(1);
    }
    if (key >= '1' && key <= '9') {
        int idx = key - '1';
        if (idx < (int)cameraDevices.size()) {
            selectedCameraIndex = idx;
            if (showCamera) {
                openSelectedCamera();
            }
        }
    }
    if (key == 'c' || key == 'C') {
        showCamera = !showCamera;
        if (showCamera) {
            refreshCameraList();
            if (!cameraDevices.empty()) {
                openSelectedCamera();
            }
        } else {
            camera.close();
        }
    }
    if (key == OF_KEY_ESC) ofExit();

    // Sensitivity hot-keys
    if (key == '+') mapper.sensitivityRMS    = std::min(3.f, mapper.sensitivityRMS + 0.1f);
    if (key == '-') mapper.sensitivityRMS    = std::max(0.2f, mapper.sensitivityRMS - 0.1f);
}

void ofApp::exit() {
    if (camera.isInitialized()) {
        camera.close();
    }
    soundStream.close();
}
