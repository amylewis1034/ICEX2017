#include <glm/glm.hpp>

#include "Particle.hpp"
#include <icex_common.hpp>
 
using namespace glm;
using namespace std;

Particle::Particle() {}

Particle::~Particle() {}

void Particle::init() {
    pTime = 0.0f;

    particles.resize(numP);
    partPositions.resize(numP);
    partColors.resize(numP);

    for (int i = 0; i < numP; ++i) {
        partPositions[i] = vec3(randFloat(-150, 50), randFloat(10, 35), randFloat(-150, 50));
        partColors[i] = vec4(0.2f, 0.2f, 0.6f, 0.2f);
    }

    //generate the VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    //generate vertex buffer to hand off to OGL - using instancing
    glGenBuffers(1, &pointsBuffer);
    //set the current state to focus on our vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, pointsBuffer);
    //actually memcopy the data - only do this once
    glBufferData(GL_ARRAY_BUFFER, numP * 3 * sizeof(float), partPositions.data(), GL_STREAM_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &colorBuffer);
    //set the current state to focus on our vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    //actually memcopy the data - only do this once
    glBufferData(GL_ARRAY_BUFFER, numP * 4 * sizeof(float), partColors.data(), GL_STREAM_DRAW);

    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Particle::rebirth(int idx) {
    partPositions[idx] = vec3(randFloat(-100, -60), randFloat(10, 35), randFloat(-150, 50));
}

void Particle::update(float dt) {
    pTime += dt;

    for (int idx = 0; idx < numP; idx++) {
        particles[idx].partVel = vec3(2 * dt, cos(pTime), 1);
        partPositions[idx] += particles[idx].partVel * dt;

        if(partPositions[idx].x > 100 || partPositions[idx].z > 100 || partPositions[idx].x < -150 || partPositions[idx].z < -150) {
            rebirth(idx);
        }
    }

    //update the GPU data
    glBindBuffer(GL_ARRAY_BUFFER, pointsBuffer);
    glBufferData(GL_ARRAY_BUFFER, numP * 3 * sizeof(float), NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * numP * 3, partPositions.data());
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glBufferData(GL_ARRAY_BUFFER, numP * 4 * sizeof(float), NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * numP * 4, partColors.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}