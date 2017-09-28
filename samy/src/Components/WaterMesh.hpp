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

struct VertexInfo {
    glm::vec3 position; float pad1;
    glm::vec3 normal; float pad2;
    glm::vec3 refracted_ray; float pad3;
};

class WaterMesh : public Component {
public:
    WaterMesh(int width=32, int height=32);
    ~WaterMesh();

    void init();
    void update(float dt);

    void draw();
    void draw_caustics();

// private:
    int width, height;
    std::vector<glm::vec3> verts;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> refracted_rays;
    std::vector<glm::vec2> texcoords;
    std::vector<glm::mat3> instance_matrices;
    std::vector<unsigned int> indices;

    std::vector<VertexInfo> vertices;

    Wave wave;

    GLuint wave_program;
    GLuint grid_vao, grid_vbo, feedback_vbo;
    
    int which = 0;
    GLBuffer vertex_buf[2];
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
    void generate_water(float t);
};

#endif