#include <GL/glew.h>

#include <iostream>
#include <vector>
#include <string>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "Mesh.hpp"
#include "Collider.hpp"

Mesh::Mesh(std::string meshname) {
    loadMesh(meshname);
}

Mesh::~Mesh() {}

void Mesh::bind() const {
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
}

void Mesh::unbind() const {
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::draw() const {
	bind();
	glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);
	unbind();
}

void Mesh::loadMesh(std::string meshname) {
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string errorString;
	bool status = tinyobj::LoadObj(shapes, materials, errorString, meshname.c_str());

	if (!status) {
		std::cerr << errorString << std::endl;
	}
	else {
		positions = shapes[0].mesh.positions;
		std::vector<float> normals = shapes[0].mesh.normals;
		std::vector<float> texcoords = shapes[0].mesh.texcoords;
		std::vector<unsigned int> indices = shapes[0].mesh.indices;

		count = indices.size();

		GLuint pBuf, nBuf, tBuf;

		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &ebo);
		glGenBuffers(1, &pBuf);
		glGenBuffers(1, &nBuf);
		glGenBuffers(1, &tBuf);

		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER, pBuf);
		glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(float), positions.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		if (!normals.empty()) {
			glBindBuffer(GL_ARRAY_BUFFER, nBuf);
			glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(1);
		}

		if (!texcoords.empty()) {
			glBindBuffer(GL_ARRAY_BUFFER, tBuf);
			glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(float), texcoords.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(2);
		}
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}

void Mesh::getCollider(Collider &collider) const {
	glm::vec3 min, max;

	for (size_t i = 0; i < positions.size(); i += 3) {
		glm::vec3 *cur = (glm::vec3 *) &positions[i];

		min = glm::min(min, *cur);
		max = glm::max(max, *cur);
	}
	collider.setMinOrig(min);
	collider.setMaxOrig(max);
}

glm::vec3 Mesh::getCenterOfMass() {
	float xS = 0, yS = 0, zS = 0;

	for (size_t i = 0; i < positions.size(); i += 3) {
		glm::vec3 *cur = (glm::vec3 *) &positions[i];
		
		xS += cur->x;
		yS += cur->y;
		zS += cur->z;
	}
	glm::vec3 com = glm::vec3(xS / positions.size(), yS / positions.size(), zS / positions.size());
	return com;
}