#pragma once

#include "ofMain.h"
#include <vector>

struct Particle {
    glm::vec2 pos;
    glm::vec2 vel;
    ofColor   color;
    float     life;
    float     size;
    float     decay;
};

class ParticleSystem {
public:
    void setup(int maxParticles = 2000);
    void update(float dt);
    void draw();

    void burst(float cx, float cy, ofColor col, float intensity, int count = 60);

    void sparkle(float cx, float cy, ofColor col, float density);

private:
    std::vector<Particle> pool;
    int   maxParticles = 2000;
    int   head         = 0;
    
    Particle makeParticle(float cx, float cy, ofColor col,
                          float speed, float size, float decay);
};
