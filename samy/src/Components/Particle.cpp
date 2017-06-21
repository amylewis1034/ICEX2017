#include <glm/glm.hpp>

#include "Particle.hpp"
#include <icex_common.hpp>
 
using namespace glm;
using namespace std;

Particle::Particle(float numP, float minX, float maxX, float minY, float maxY, float minZ, float maxZ, float r, float g, float b, float a) :
    numP(numP),
    minX(minX),
    maxX(maxX),
    minY(minY),
    maxY(maxY),
    minZ(minZ),
    maxZ(maxZ),
    r(r),
    g(g),
    b(b),
    a(a)
{}

Particle::Particle(float numP, float minX, float maxX, float minY, float maxY, float minZ, float maxZ) :
    numP(numP),
    minX(minX),
    maxX(maxX),
    minY(minY),
    maxY(maxY),
    minZ(minZ),
    maxZ(maxZ),
    r(1),
    g(1),
    b(1),
    a(1)
{}

Particle::~Particle() {}

void Particle::init() {
    pTime = 0.0f;

    particles.resize(numP);
    partPositions.resize(numP);
    partColors.resize(numP);

    for (int i = 0; i < numP;) {
        if (numP == 300) {
            partPositions[i] = vec3(randFloat(minX, maxX), randFloat(minY + 5, maxY), randFloat(minZ, maxZ));
            partColors[i] = vec4(r, g, b, a);
            
            i++;
        } else {
            float tmpX = randFloat(minX, maxX);
            float tmpY = randFloat(-5, 0);
            float tmpZ = randFloat(minZ, maxZ);

            partPositions[i + 0] = vec3(tmpX + randFloat(-1, 1), tmpY + randFloat(-1, 1), tmpZ + randFloat(-1, 1));
            partPositions[i + 1] = vec3(tmpX + randFloat(-1, 1), tmpY + randFloat(-1, 1), tmpZ + randFloat(-1, 1));
            partPositions[i + 2] = vec3(tmpX + randFloat(-1, 1), tmpY + randFloat(-1, 1), tmpZ + randFloat(-1, 1));
            partPositions[i + 3] = vec3(tmpX + randFloat(-1, 1), tmpY + randFloat(-1, 1), tmpZ + randFloat(-1, 1));
            partPositions[i + 4] = vec3(tmpX + randFloat(-1, 1), tmpY + randFloat(-1, 1), tmpZ + randFloat(-1, 1));
            partPositions[i + 5] = vec3(tmpX + randFloat(-1, 1), tmpY + randFloat(-1, 1), tmpZ + randFloat(-1, 1));
         
            partColors[i + 0] = vec4(r, g, b, a);
            partColors[i + 1] = vec4(r, g, b, a);
            partColors[i + 2] = vec4(r, g, b, a);
            partColors[i + 3] = vec4(r, g, b, a);
            partColors[i + 4] = vec4(r, g, b, a);
            partColors[i + 5] = vec4(r, g, b, a);
        
            i += 6;
        }
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
    if (numP == 300) {
        partPositions[idx] = vec3(randFloat(minX, maxX - 100), randFloat(minY, maxY), randFloat(minZ, maxZ));
    }
    else {
        float tmpX = randFloat(minX, maxX);
        float tmpY = randFloat(-5, 0);
        float tmpZ = randFloat(minZ, maxZ);

        partPositions[idx + 0] = vec3(tmpX + randFloat(-1, 1), tmpY + randFloat(-1, 1), tmpZ + randFloat(-1, 1));
        partPositions[idx + 1] = vec3(tmpX + randFloat(-1, 1), tmpY + randFloat(-1, 1), tmpZ + randFloat(-1, 1));
        partPositions[idx + 2] = vec3(tmpX + randFloat(-1, 1), tmpY + randFloat(-1, 1), tmpZ + randFloat(-1, 1));
        partPositions[idx + 3] = vec3(tmpX + randFloat(-1, 1), tmpY + randFloat(-1, 1), tmpZ + randFloat(-1, 1));
        partPositions[idx + 4] = vec3(tmpX + randFloat(-1, 1), tmpY + randFloat(-1, 1), tmpZ + randFloat(-1, 1));
        partPositions[idx + 5] = vec3(tmpX + randFloat(-1, 1), tmpY + randFloat(-1, 1), tmpZ + randFloat(-1, 1));
    }
}

void Particle::update(float dt) {
    pTime += dt;

    for (int idx = 0; idx < numP;) {
        if (numP == 300) {
            particles[idx].partVel = vec3(2 * dt, cos(pTime), 1);

             partPositions[idx] += particles[idx].partVel * dt;

            if(partPositions[idx].x < minX ||  partPositions[idx].x > maxX || partPositions[idx].y < minY|| partPositions[idx].y > maxY ||partPositions[idx].z < minZ || partPositions[idx].z > maxZ ) {
                rebirth(idx);
            }

            idx++;
        }
        else {
            for (int g = 0; g < 6; g++) {
                particles[idx + g].partVel = vec3(2 * dt, 10 / randFloat(4,6), 1);

                partPositions[idx + g] += particles[idx].partVel * dt;

                if(partPositions[idx + g].x < minX ||  partPositions[idx + g].x > maxX || partPositions[idx + g].y > maxY ||partPositions[idx + g].z < minZ || partPositions[idx + g].z > maxZ ) {
                    rebirth(idx + 0);
                    rebirth(idx + 1);
                    rebirth(idx + 2);
                    rebirth(idx + 3);
                    rebirth(idx + 4);
                    rebirth(idx + 5);
                    g = 6;
                }
            }
            idx += 6;
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