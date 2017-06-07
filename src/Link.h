#pragma once
#ifndef _LINK_H_
#define _LINK_H_

#include <vector>

#include "Shape.h"
#include "MatrixStack.h"

using namespace std;
using namespace Eigen;

class Link
{
public:
	Link(shared_ptr<Shape> linkShape);
	virtual ~Link();
	void addChild(shared_ptr<Link> childLink);
	void draw(const shared_ptr<Program> prog, const shared_ptr<MatrixStack> M) const;

	void scale(float angle);
	void rotate(float angle, Vector3f axis);
	void translate(Vector3f trans);
	void pushMatrix();
	void popMatrix();

	void scaleParent(float angle);
	void rotateParent(float angle, Vector3f axis);
	void translateParent(Vector3f trans);
	void pushParentMatrix();
	void popParentMatrix();

	void centerMeshGeometry();
	void scaleMesh(float scale);
	void rotateMesh(float angle, Vector3f axis);
	void translateMesh(Vector3f trans);

private:
	shared_ptr<Shape> shape;
	vector<shared_ptr<Link>> children;

	shared_ptr<MatrixStack> pE;  // Bind transform with respect to parent.
	shared_ptr<MatrixStack> mE;  // Transform with respect to mesh origin.
	shared_ptr<MatrixStack> E;  // Global transform.
};

#endif
