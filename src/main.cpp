#include "ofMain.h"
#include "ofApp.h"

int main() {
    // Request an OpenGL 3.2 core-profile context
    ofGLFWWindowSettings settings;
    settings.setGLVersion(3, 2);
    settings.setSize(1280, 720);
    settings.windowMode = OF_WINDOW;
    settings.title      = "Guitar Visualizer";

    auto window = ofCreateWindow(settings);
    ofRunApp(window, std::make_shared<ofApp>());
    ofRunMainLoop();

    return 0;
}
