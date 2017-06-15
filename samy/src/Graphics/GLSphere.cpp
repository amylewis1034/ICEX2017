#include <GL/glew.h>
#include "GLSphere.hpp"
#include <Components/Mesh.hpp>
#include <icex_common.hpp>

Mesh *GLSphere::mesh = nullptr;

void GLSphere::init() {
	if (mesh != nullptr) return;

	mesh = new Mesh(RESOURCE_PATH "objs/sphere.obj");
}

void GLSphere::draw() {
	mesh->draw();
}