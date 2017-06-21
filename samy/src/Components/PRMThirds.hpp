#ifndef PRMTHIRDS_HPP
#define PRMTHIRDS_HPP

#include <GL/glew.h>

#include "Component.hpp"
#include "Transform.hpp"

#include <PRM/PRMNode.h>

class PRMThirds : public Component {
public:
    PRMThirds();
    ~PRMThirds();

    void init();
    void update(float dt);
    void postrender();

private:
    void setCamPos6dof(const glm::vec3 pos, const glm::vec3 dir);

    Transform *transform;
    glm::vec3 camPos, camDir;

    PRMNode *curNode, *bestRootNode = nullptr;
    double bestRootWeight = 0.0;
    int rootIter = 0;
    bool generatingRootNode = true;
    bool genThirds = true;

    GLuint framebuffer, renderTexture, depthRenderBuffer;
    int actualWidth, actualHeight;
};

#endif