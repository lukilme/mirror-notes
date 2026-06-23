#pragma once

#include "ofMain.h"
#include <vector>

struct Particle {
    glm::vec2 pos;
    glm::vec2 vel;
    ofColor   color;
    float     life;       // 0..1 (1=just born, 0=dead)
    float     size;
    float     decay;      // life lost per second
};

class ParticleSystem {
public:
    void setup(int maxParticles = 2000);
    void update(float dt);
    void draw();

    // Spawn a burst at (cx,cy) with given colour intensity
    void burst(float cx, float cy, ofColor col, float intensity, int count = 60);

    // Spawn continuous stream (treble sparkles at top)
    void sparkle(float cx, float cy, ofColor col, float density);

private:
    std::vector<Particle> pool;
    int   maxParticles = 2000;
    int   head         = 0;   // ring-buffer head for O(1) spawn

    Particle makeParticle(float cx, float cy, ofColor col,
                          float speed, float size, float decay);
};
