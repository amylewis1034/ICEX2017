#pragma once
#ifndef _BUBBLES_H_
#define _BUBBLES_H_

#include "Link.h"

using namespace std;
using namespace Eigen;

class BubblesShapes {
public:
	BubblesShapes();
	BubblesShapes(string RESOURCE_DIR);
	~BubblesShapes();
	shared_ptr<Shape> sphere;
};

class Bubbles {
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	Bubbles();
	Bubbles(shared_ptr<BubblesShapes> sharkShapes);
	~Bubbles();

	void draw(const shared_ptr<Program> prog, double t);

private:
	float count;
	float speed;
	float yOffset;
	Vector2f yRange;
	Vector2f xRange;
	Vector2f zRange;

	shared_ptr<BubblesShapes> shapes;
	shared_ptr<Link> parentLink;
	shared_ptr<MatrixStack> M;
};

#endif
