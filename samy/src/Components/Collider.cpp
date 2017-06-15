#include "Collider.hpp"

#include <glm/gtc/matrix_access.hpp>
#include <Components/Mesh.hpp>
#include <Components/Transform.hpp>

#include <glm/gtx/string_cast.hpp>
#include <iostream>


Collider::Collider()
	: minOrig(glm::vec3(0.0f)), maxOrig(glm::vec3(0.0f)), collided(false)
{}
Collider::Collider(glm::vec3 min, glm::vec3 max)
	: minOrig(min), maxOrig(max), collided(false)
{}
Collider::Collider(const Mesh &mesh) :
	collided(false)
{
	mesh.getCollider(*this);
}

Collider::~Collider() {}

void Collider::init() {
	Transform *t = gameobject->getComponent<Transform>();
	this->transform(t->getMatrix());
	isDirty = false;

	/* Get collider from mesh if none specified yet */
	if (minOrig == glm::vec3(0.0f) && maxOrig == glm::vec3(0.0f)) {
		Mesh *m = gameobject ->getComponent<Mesh>();
		m->getCollider(*this);
	}
}

void Collider::update(float dt) {
	isDirty = true;
}

// algorithm from http://dev.theomader.com/transform-bounding-boxes/
// translated Eigen to glm from my 473 code
void Collider::transform(const glm::mat4 &ctm) {
	glm::vec3 right = glm::vec3(glm::column(ctm, 0));
	glm::vec3 up = glm::vec3(glm::column(ctm, 1));
	glm::vec3 back = glm::vec3(glm::column(ctm, 2));
	glm::vec3 trans = glm::vec3(glm::column(ctm, 3));

	glm::vec3 xa = right * this->minOrig[0];
	glm::vec3 xb = right * this->maxOrig[0];

	glm::vec3 ya = up * this->minOrig[1];
	glm::vec3 yb = up * this->maxOrig[1];

	glm::vec3 za = back * this->minOrig[2];
	glm::vec3 zb = back * this->maxOrig[2];

	this->min = glm::min(xa, xb) + glm::min(ya, yb) + glm::min(za, zb) + trans;
	this->max = glm::max(xa, xb) + glm::max(ya, yb) + glm::max(za, zb) + trans;
}

bool Collider::hit(const Collider &other) {
	if (isDirty) {
		Transform *t = gameobject->getComponent<Transform>();
		this->transform(t->getMatrix());
		isDirty = false;
	}

	return min.x <= other.max.x && max.x >= other.min.x &&
		min.y <= other.max.y && max.y >= other.min.y &&
		min.z <= other.max.z && max.z >= other.min.z;
}

void Collider::setMinOrig(glm::vec3 min) {
	this->minOrig = min;
}

void Collider::setMaxOrig(glm::vec3 max) {
	this->maxOrig = max;
}
