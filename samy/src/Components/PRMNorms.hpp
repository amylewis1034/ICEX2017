#ifndef PRMNORMS_HPP
#define PRMNORMS_HPP

#include <GL/glew.h>

#include "Component.hpp"
#include "Transform.hpp"

#include <PRM/PRMNode.h>

class PRMNorms : public Component {
public:
    PRMNorms(int numNodes);
    ~PRMNorms();

    void init();
    void update(float dt);
    void postrender(const glm::mat4 &projection, const glm::mat4 &view);

private:
    void setCamPos6dof(const glm::vec3 pos, const glm::vec3 dir);

    Transform *transform;
    glm::vec3 camPos, camDir;

    PRMNode *curNode, *bestRootNode = nullptr;
    double bestRootWeight = 0.0;
    int rootIter = 0;
    bool generatingRootNode = true;
    bool genThirds = false;

    int numNodes;

    GLuint framebuffer, renderTexture, depthRenderBuffer;
    int actualWidth, actualHeight;
};

#endif