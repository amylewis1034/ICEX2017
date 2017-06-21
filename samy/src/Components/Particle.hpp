#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include "Component.hpp"

struct particle_t {
    float charge = 1.0f;
    float mass = 1.0f;
    glm::vec3 partVel = glm::vec3(0.0f, 0.0f, 0.0f);
    float scale = 1.0f;
};

class Particle : public Component {
public:
    Particle(float numP, float minX, float maxX, float minY, float maxY, float minZ, float maxZ, float r, float g, float b, float a);
    Particle(float numP, float minX, float maxX, float minY, float maxY, float minZ, float maxZ);
    virtual ~Particle();

    virtual void init();
    virtual void update(float dt);

    GLuint VAO;
    float numP;
    
 private:
    std::vector<particle_t> particles;
    std::vector<glm::vec3> partPositions;
    std::vector<glm::vec4> partColors;

    float pTime = 0.0f; //reset in init

    void rebirth(int idx);

    GLuint pointsBuffer;
    GLuint colorBuffer;

    float minX, maxX, minY, maxY, minZ, maxZ, r, g, b, a;
};

#endif