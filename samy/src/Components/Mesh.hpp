#ifndef MESH_HPP
#define MESH_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <string>
#include <vector>

#include "Component.hpp"
#include "Collider.hpp"

class Mesh : public Component {
public:
    Mesh(std::string meshname);
    ~Mesh();

    void bind() const;
    void unbind() const;
    void draw() const;

    void getCollider(Collider &collider) const;
    std::vector<float> getPositions() { return positions; }
    glm::vec3 getCenterOfMass();

private:
    GLuint vao, ebo;
    GLsizei count;
    std::vector<float> positions;

    void loadMesh(std::string meshname);
};

#endif