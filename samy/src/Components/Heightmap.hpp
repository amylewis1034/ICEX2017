#ifndef HEIGHTMAP_HPP
#define HEIGHTMAP_HPP

#include "Component.hpp"
#include <Graphics/GLVertexArrayObject.hpp>
#include <Graphics/GLBuffer.hpp>
#include <glm/glm.hpp>
#include <string>
#include <vector>

class Heightmap : public Component {
public:
    Heightmap();
    ~Heightmap();

    void init();
    void update(float dt);

    void loadFromFile(const std::string &filename);
    void draw();

	float getHeightAt(float x, float z) const;

private:
	int width, height;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;
    std::vector<unsigned int> indices;

    GLVertexArrayObject vao;
    GLBuffer ebo;
};

#endif