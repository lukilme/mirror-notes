#include "ParticleSystem.h"
#include <cmath>

void ParticleSystem::setup(int max) {
    maxParticles = max;
    pool.resize(maxParticles);
    for (auto& p : pool) p.life = 0.f;
}

void ParticleSystem::update(float dt) {
    for (auto& p : pool) {
        if (p.life <= 0.f) continue;
        p.life -= p.decay * dt;
        if (p.life < 0.f) { p.life = 0.f; continue; }

        // Gravity + drag
        p.vel.y   += 60.f * dt;   // gentle gravity
        p.vel     *= (1.f - 2.f * dt);
        p.pos     += p.vel * dt;
    }
}

void ParticleSystem::draw() {
    ofEnableBlendMode(OF_BLENDMODE_ADD); // additive for glow effect
    for (const auto& p : pool) {
        if (p.life <= 0.f) continue;
        float alpha = p.life * 255.f;
        ofSetColor(p.color.r, p.color.g, p.color.b, (int)alpha);
        ofDrawCircle(p.pos.x, p.pos.y, p.size * p.life);
    }
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
}

void ParticleSystem::burst(float cx, float cy, ofColor col,
                           float intensity, int count) {
    int n = int(count * intensity);
    for (int i = 0; i < n; ++i) {
        float speed = ofRandom(80.f, 400.f) * intensity;
        float angle = ofRandom(0.f, TWO_PI);
        Particle& p = pool[head % maxParticles];
        p = makeParticle(cx, cy, col, speed, ofRandom(2.f, 6.f), ofRandom(0.6f, 1.4f));
        p.vel = {std::cos(angle)*speed, std::sin(angle)*speed};
        head++;
    }
}

void ParticleSystem::sparkle(float cx, float cy, ofColor col, float density) {
    int n = int(density * 4.f);
    for (int i = 0; i < n; ++i) {
        float speed = ofRandom(20.f, 120.f);
        Particle& p = pool[head % maxParticles];
        p = makeParticle(cx + ofRandom(-200.f, 200.f), cy,
                         col, speed, ofRandom(1.f, 3.f), ofRandom(1.f, 2.5f));
        p.vel = {ofRandom(-40.f, 40.f), -speed};
        head++;
    }
}

Particle ParticleSystem::makeParticle(float cx, float cy, ofColor col,
                                       float /*speed*/, float sz, float decay) {
    Particle p;
    p.pos   = {cx, cy};
    p.vel   = {0.f, 0.f};
    p.color = col;
    p.life  = 1.f;
    p.size  = sz;
    p.decay = decay;
    return p;
}
