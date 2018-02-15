#ifndef VIEWFRUSTRUM_HPP
#define VIEWFRUSTRUM_HPP

#include <glm/glm.hpp>

class ViewFrustrum {
public:
    ViewFrustrum();
    ~ViewFrustrum();

    void extractPlanes(glm::mat4 P, glm::mat4 V);
    void printPlanes();

// private:
    glm::vec4 Left, Right, Bottom, Top, Near, Far;
    glm::vec4 planes[6];
};

#endif