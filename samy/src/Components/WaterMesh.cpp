#include "WaterMesh.hpp"
#include "Transform.hpp"
#include <GameObject.hpp>
#include <World.hpp>

#include <fstream>
#include <iostream>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <limits>

extern World *world;

const float left = -10.0f, right = 10.0f;
const float bot = -10.0f, top = 10.0f;

// static float single_wave(float x, float y, float t, const Wave &wave) {
//     float theta = glm::dot(wave.direction, glm::vec2(x, y));
//     return 15.0f + wave.amplitude * glm::sin(theta * wave.frequency + t * wave.phase);
// }

// static glm::vec3 wave_normal(float x, float y, float t, const Wave &wave) {
//     float theta = glm::dot(wave.direction, glm::vec2(x, y));
//     float dx = wave.amplitude * wave.direction.x * wave.frequency
//         * glm::cos(theta * wave.frequency + t * wave.phase);
//     float dy = wave.amplitude * wave.direction.y * wave.frequency
//         * glm::cos(theta * wave.frequency + t * wave.phase);
    
//     return glm::normalize(glm::vec3(-dx, 1.0f, -dy));
// }
static float single_wave(float x, float y, float t, const Wave &wave) {
    float theta = glm::sqrt(glm::dot(glm::vec2(x, y), glm::vec2(x, y)));
    return 25.0f + wave.amplitude * glm::cos(theta * wave.frequency + t * wave.phase);
}

static glm::vec3 wave_normal(float x, float y, float t, const Wave &wave) {
    float theta = glm::sqrt(glm::dot(glm::vec2(x, y), glm::vec2(x, y)));
    float dx = wave.amplitude * wave.direction.x * wave.frequency * x
        * glm::sin(theta * wave.frequency + t * wave.phase) / theta;
    float dy = wave.amplitude * wave.direction.y * wave.frequency * y
        * glm::sin(theta * wave.frequency + t * wave.phase) / theta;
    
    return glm::normalize(glm::vec3(-dx, 1.0f, -dy));
}

WaterMesh::WaterMesh(int width, int height) :
    width(width), height(height), wave({0.5f, 4.0f, 1.0f, glm::vec2(1.0f, 0.0f)}) {}

WaterMesh::~WaterMesh() {}

void WaterMesh::init() {
    vertices.resize(width * height);
    texcoords.resize(width * height);
    normals.resize(width * height, glm::vec3(0));
    refracted_rays.resize(width * height);

    generate_vertices(0.0f, world->getMainlightPosition());
    generate_indices();

    bboxes.resize(indices.size() - 2);
    instance_matrices.resize(bboxes.size());
    generate_bboxes();

    cubemesh = new Mesh(RESOURCE_PATH "objs/cube.obj");
    assert(cubemesh);

    init_buffers();

}

void WaterMesh::update(float dt) {
    static float t = 0.0f;
    t += dt;

    generate_vertices(t, world->getMainlightPosition());
    generate_bboxes();

    update_buffers();
}

void WaterMesh::draw() {
    glDisable(GL_CULL_FACE);
    vao.bind();
    ebo.bind();
    glDrawElements(GL_TRIANGLE_STRIP, indices.size(), GL_UNSIGNED_INT, 0);
    ebo.unbind();
    vao.unbind();
    glEnable(GL_CULL_FACE);
}

void WaterMesh::draw_caustics() {
    cubemesh->draw_instanced(indices.size() - 2);
}

float WaterMesh::getHeightAt(float x, float z) const {
	assert(gameobject != nullptr);

	Transform *t = this->gameobject->getComponent<Transform>();

	assert(t != nullptr);

	const glm::vec3 &pos = t->getPosition();
	const glm::vec3 &scale = t->getScale();

	/* Scale to between -1 and 1 */
	float scaled_x = x / scale.x;
	float scaled_z = z / scale.z;

	int i = (this->width - 1) * ((scaled_x + 1) / 2.0f);
	int j = (this->height - 1) * ((scaled_z + 1) / 2.0f);

	float offset = vertices[j * this->height + i].y;
	return pos.y + offset * scale.y;
}

void WaterMesh::generate_vertices(float t, const glm::vec3 &light_position) {
    const float n_air = 1.0f, n_water = 1.33f;

    /* Create vertices and texture coordinates */
    float xoffset = (right - left) / (width - 1);
    float yoffset = (top - bot) / (height - 1);

    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            int index = j * width + i;
            float x = i * xoffset + left;
            float y = j * yoffset + bot;

            glm::vec3 &v = vertices[index];
            glm::vec3 &n = normals[index];
            glm::vec3 &r = refracted_rays[index];

            float height = 0.0f;
            height = single_wave(x, y, t, this->wave);

            v = glm::vec3(x, height, y);
            n = wave_normal(x, y, t, this->wave);

            glm::vec3 incident = glm::normalize(v - light_position);
            r = glm::refract(incident, n, n_air / n_water);

			texcoords[index] = glm::vec2((float)i / width, 1.0f - (float)j / height);
            // texcoords[j * width + i] *= this->texture_scale;
        }
    }
}

void WaterMesh::generate_indices() {
    /* Create indices for rendering */
    for (int j = 0; j < height - 1; j++) {
        if (j % 2 == 0) {
            /* Even row: left to right */
            for (int i = 0; i < width; i++) {
                indices.push_back(j * width + i);
                indices.push_back((j + 1) * width + i);
            }

            if (j < height - 2) {
                indices.push_back((j + 1) * width + width - 1);
            }
        }
        else {
            /* Odd row: right to left */
            for (int i = width - 1; i >= 0; i--) {
                indices.push_back(j * width + i);
                indices.push_back((j + 1) * width + i);
            }

            if (j < height - 2) {
                indices.push_back((j + 1) * width);
            }
        }
    }
}

void WaterMesh::generate_bboxes() {
    glm::vec3 a[2], b[2], c[2];
    a[0] = vertices[indices[0]];
    a[1] = vertices[indices[0]] - refracted_rays[indices[0]] * (a[0].y / refracted_rays[indices[0]].y);
    b[0] = vertices[indices[1]];
    b[1] = vertices[indices[1]] - refracted_rays[indices[1]] * (b[0].y / refracted_rays[indices[1]].y);
    for (int i = 0; i < bboxes.size(); i++) {
        // vol_verts[(i + 4) % 6] = vertices[indices[i + 2]];
        // vol_verts[(i + 5) % 6] = vertices[indices[i + 2]] + refracted_rays[indices[i + 2]];
        c[0] = vertices[indices[i + 2]];
        c[1] = vertices[indices[i + 2]] - refracted_rays[indices[i + 2]] * (c[0].y / refracted_rays[indices[i + 2]].y);
    
        const glm::vec3 vol_verts[6] {
            a[0], a[1], b[0], b[1], c[0], c[1]
        };

        glm::vec3 min_point {std::numeric_limits<float>::max()};
        glm::vec3 max_point {std::numeric_limits<float>::min()};
        for (const auto &v : vol_verts) {
            min_point = glm::min(min_point, v);
            max_point = glm::max(max_point, v);
        }

        bboxes[i].scale = max_point - min_point;
        bboxes[i].center = (min_point + max_point) / 2.0f;

        // TODO: could access gameobject to get transforms
        glm::mat4 t, s;
        t = glm::translate(t, bboxes[i].center);
        s = glm::scale(s, bboxes[i].scale);
        instance_matrices[i] = t * s;  

        a[0] = b[0];
        a[1] = b[1];
        b[0] = c[0];
        b[1] = c[1];
    }
}

void WaterMesh::init_buffers() {
    /* For drawing stuff */
    positions.loadData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STREAM_DRAW);

    nBuf.loadData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STREAM_DRAW);

	tBuf.loadData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(glm::vec2), texcoords.data(), GL_STATIC_DRAW);

    this->instanceBuf.loadData(GL_ARRAY_BUFFER, this->instance_matrices.size() * sizeof(glm::mat3), this->instance_matrices.data(), GL_STREAM_DRAW);    

    this->instanceIndicesBuf.loadData(GL_ARRAY_BUFFER, indices.size() * sizeof(unsigned), indices.data(), GL_STATIC_DRAW);

    vao.bind();

    positions.bind();
    vao.addAttribute(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    nBuf.bind();
    vao.addAttribute(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	tBuf.bind();
	vao.addAttribute(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

    vao.unbind();

    ebo.loadData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    /* Instanced caustic vao */
    glBindVertexArray(cubemesh->getVAO());
    this->instanceBuf.bind();
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::mat3), (GLvoid *)0);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(glm::mat3), (GLvoid *)sizeof(glm::vec3));
    glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(glm::mat3), (GLvoid *)(2 * sizeof(glm::vec3)));
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);

    this->instanceIndicesBuf.bind();
    glEnableVertexAttribArray(6);
    glVertexAttribIPointer(6, 3, GL_UNSIGNED_INT, sizeof(unsigned int), 0);
    glVertexAttribDivisor(6, 1);

    glBindVertexArray(0);

    /* Caustic block */
    uniform_block.loadData(GL_UNIFORM_BUFFER, 3 * vertices.size() * sizeof(glm::vec4), nullptr, GL_STREAM_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniform_block.getHandle());
    for (int i = 0; i < vertices.size(); i++) {
        glBufferSubData(GL_UNIFORM_BUFFER, 48 * i, 12, glm::value_ptr(vertices[i]));
        glBufferSubData(GL_UNIFORM_BUFFER, 48 * i + 16, 12, glm::value_ptr(normals[i]));
        glBufferSubData(GL_UNIFORM_BUFFER, 48 * i + 32, 12, glm::value_ptr(refracted_rays[i]));
    }

}

void WaterMesh::update_buffers() {
    positions.loadData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STREAM_DRAW);
    nBuf.loadData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STREAM_DRAW);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniform_block.getHandle());
    for (int i = 0; i < vertices.size(); i++) {
        glBufferSubData(GL_UNIFORM_BUFFER, 48 * i, 12, glm::value_ptr(vertices[i]));
        glBufferSubData(GL_UNIFORM_BUFFER, 48 * i + 16, 12, glm::value_ptr(normals[i]));
        glBufferSubData(GL_UNIFORM_BUFFER, 48 * i + 32, 12, glm::value_ptr(refracted_rays[i]));
    }

    this->instanceBuf.loadData(GL_ARRAY_BUFFER, this->instance_matrices.size() * sizeof(glm::mat3), this->instance_matrices.data(), GL_STREAM_DRAW);    
}
