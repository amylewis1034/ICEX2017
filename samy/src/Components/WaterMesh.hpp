#ifndef WATERMESH_HPP
#define WATERMESH_HPP

#include "Component.hpp"
#include "Mesh.hpp"
#include <Graphics/GLVertexArrayObject.hpp>
#include <Graphics/GLBuffer.hpp>
#include <glm/glm.hpp>
#include <string>
#include <vector>

struct Wave {
    float amplitude, frequency, phase;
    glm::vec2 direction;
};

struct BoundingBox {
    glm::vec3 center, scale;
};

class WaterMesh : public Component {
public:
    WaterMesh(int width=32, int height=32);
    ~WaterMesh();

    void init();
    void update(float dt);

    void draw();
    void draw_caustics();

    float getHeightAt(float x, float z) const;

// private:
    int width, height;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> refracted_rays;
    std::vector<glm::vec2> texcoords;
    std::vector<unsigned int> indices;
    std::vector<glm::mat4> instance_matrices;
    Wave wave;

    GLBuffer positions, nBuf, tBuf, instanceBuf, instanceIndicesBuf;
    GLVertexArrayObject vao;
    GLBuffer ebo;
    GLBuffer uniform_block;

    std::vector<BoundingBox> bboxes;

    Mesh *cubemesh;

    void generate_vertices(float t, const glm::vec3 &light_position);
    void generate_indices();
    void generate_bboxes();
    void init_buffers();
    void update_buffers();
};

#endif
