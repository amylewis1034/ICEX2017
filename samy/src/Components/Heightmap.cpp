#include "Heightmap.hpp"
#include "Transform.hpp"
#include <GameObject.hpp>

#include <fstream>
#include <iostream>
#include <glm/gtx/string_cast.hpp>

#include <stb_image.h>

const float left = -1.0f, right = 1.0f;
const float bot = -1.0f, top = 1.0f;

Heightmap::Heightmap(float texture_scale) :
    texture_scale(texture_scale)
{

}

Heightmap::~Heightmap() {

}

void Heightmap::init() {}
void Heightmap::update(float dt) {}

void Heightmap::loadFromFile(const std::string &filename) {
    /* Read in image */
	int width, height, channels;
    unsigned char *image = stbi_load(filename.c_str(), &width, &height, &channels, 1);
    if (image == NULL) {
        std::cout
            << "ERROR::TEXTURE::LOAD_FAILED::"
            << filename
            << std::endl;
    }

	this->width = width;
	this->height = height;

    vertices.resize(width * height);
	texcoords.resize(width * height);

    /* Create vertices and texture coordinates */
    float xoffset = (right - left) / (width - 1);
    float yoffset = (top - bot) / (height - 1);

    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            unsigned char *pixel = &image[j * width + i];

            vertices[j * width + i] = glm::vec3(
                i * xoffset + left,
                *pixel / 255.0f,
                j * yoffset + bot
            );

			texcoords[j * width + i] = glm::vec2(
				(float)i / width,
				1.0f - (float)j / height
            );
            texcoords[j * width + i] *= this->texture_scale;
        }
    }

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

    /* Generate normals */
    normals.resize(vertices.size(), glm::vec3(0));
    for (int i = 0; i < indices.size() - 2; i++) {
        glm::vec3 &v0 = vertices[indices[i]];
        glm::vec3 &v1 = vertices[indices[i + 1]];
        glm::vec3 &v2 = vertices[indices[i + 2]];

        glm::vec3 n = glm::cross(v1 - v0, v2 - v0);

        if ((indices[i] / width) % 2 == 0) {
			normals[indices[i]] += n;
			normals[indices[i + 1]] += n;
			normals[indices[i + 2]] += n;
        }
        else {
			normals[indices[i]] -= n;
			normals[indices[i + 1]] -= n;
			normals[indices[i + 2]] -= n;
        }

    }

    for (int i = 0; i < normals.size(); i++) {
        normals[i] = glm::normalize(normals[i]);
    }

    GLBuffer positions;
    positions.loadData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

    GLBuffer nBuf;
    nBuf.loadData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);

	GLBuffer tBuf;
	tBuf.loadData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(glm::vec2), texcoords.data(), GL_STATIC_DRAW);

    vao.bind();

    positions.bind();
    vao.addAttribute(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    nBuf.bind();
    vao.addAttribute(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	tBuf.bind();
	vao.addAttribute(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

    vao.unbind();

    ebo.loadData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    stbi_image_free(image);
}

void Heightmap::draw() {
    vao.bind();
    ebo.bind();
    glDrawElements(GL_TRIANGLE_STRIP, indices.size(), GL_UNSIGNED_INT, 0);
    ebo.unbind();
    vao.unbind();
}

float Heightmap::getHeightAt(float x, float z) const {
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