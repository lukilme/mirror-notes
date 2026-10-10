#pragma once

#include "ofMain.h"
#include "Logger.h"
#include <vector>

struct Particle {
    glm::vec2 pos;
    glm::vec2 vel;
    ofColor   color;
    float     life;
    float     size;
    float     decay;
};

enum class ParticleType {
    Burst,
    Sparkle,
    Glow,
};

class ParticleFactory {
public:
    static Particle create(ParticleType type,
                           float cx, float cy,
                           const ofColor& col,
                           float speed,
                           float size,
                           float decay) {
        Particle p;
        p.pos = {cx, cy};
        p.vel = {0.f, 0.f};
        p.color = col;
        p.life = 1.f;
        p.size = size;
        p.decay = decay;

        switch (type) {
            case ParticleType::Burst:
                p.life = 1.f;
                p.size = size;
                p.decay = decay;
                break;
            case ParticleType::Sparkle:
                p.life = 1.2f;
                p.size = size * 0.8f;
                p.decay = decay * 1.2f;
                break;
            case ParticleType::Glow:
                p.life = 1.6f;
                p.size = size * 1.4f;
                p.decay = decay * 0.8f;
                break;
        }

        (void)speed;
        return p;
    }
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

    Particle makeParticle(ParticleType type,
                          float cx, float cy,
                          ofColor col,
                          float speed,
                          float size,
                          float decay);
};
