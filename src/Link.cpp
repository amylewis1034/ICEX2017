#include "Link.h"
#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Program.h"

using namespace std;
using namespace Eigen;

Link::Link(shared_ptr<Shape> linkShape) {
	pE = make_shared<MatrixStack>();
	mE = make_shared<MatrixStack>();
	E = make_shared<MatrixStack>();
	shape = linkShape;
}

Link::~Link() {
}

void Link::addChild(shared_ptr<Link> childLink) {
	children.push_back(childLink);
}

void Link::draw(const shared_ptr<Program> prog, const shared_ptr<MatrixStack> M) const
{
	M->pushMatrix();
	M->multMatrix(E->topMatrix());
	M->multMatrix(pE->topMatrix());
	M->pushMatrix();
	M->multMatrix(mE->topMatrix());
	glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());
	shape->draw(prog);
	M->popMatrix();
	for(auto childPtr = children.begin(); childPtr != children.end(); ++childPtr) {
	  shared_ptr<Link> child = *childPtr;
	  child->draw(prog, M);
	}
	M->popMatrix();
}

void Link::translate(Vector3f trans) {
	E->translate(trans);
}

void Link::rotate(float angle, Vector3f axis) {
	E->rotate(angle, axis);
}

void Link::pushMatrix() {
	E->pushMatrix();
}

void Link::popMatrix() {
	E->popMatrix();
}

void Link::scale(float scale) {
	E->scale(scale);
}

void Link::translateParent(Vector3f trans) {
	pE->translate(trans);
}

void Link::rotateParent(float angle, Vector3f axis) {
	pE->rotate(angle, axis);
}

void Link::scaleParent(float scale) {
	pE->scale(scale);
}

void Link::pushParentMatrix() {
	pE->pushMatrix();
}

void Link::popParentMatrix() {
	pE->popMatrix();
}

void Link::centerMeshGeometry() {
	Vector3f center = shape->getCenter();
	mE->translate(-center);
}

void Link::translateMesh(Vector3f trans) {
	mE->translate(trans);
}

void Link::rotateMesh(float angle, Vector3f axis) {
	mE->rotate(angle, axis);
}

void Link::scaleMesh(float scale) {
	mE->scale(scale);
}


