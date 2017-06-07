#include "Bubbles.h"
#include "Utilities.h"

BubblesShapes::BubblesShapes() {

}

// A class to hold shapes for the bubbles
BubblesShapes::BubblesShapes(string RESOURCE_DIR) {
	sphere = make_shared<Shape>();
	sphere->loadMesh(RESOURCE_DIR + "sphere.obj");
	sphere->resize();
	sphere->init();
}

BubblesShapes::~BubblesShapes() {

}

Bubbles::Bubbles() {
}

Bubbles::Bubbles(shared_ptr<BubblesShapes> BubbleShapes) {

	// Initialize meaningful random bubbles.
	M = make_shared<MatrixStack>();
	count = -1;
	speed = 2.0f;

	// Let the random starting range be the whole world
	yRange << -1.0f, 35.0f;
	xRange << -400.0f, 400.0f;
	zRange << -400.0f, 400.0f;

	// Have a random y offset so not all bubbles share the same height
	yOffset = randRangef(yRange[0], yRange[1]);

	// Record the bubble shapes
	// I realize this is non-sensicle, but, at the time I was patterning off of the shark class
	shapes = BubbleShapes;

	M->pushMatrix();
	M->translate(Vector3f(randRangef(xRange[0], xRange[1]), 0.0f, randRangef(zRange[0], zRange[1])));

	// Generate a center sphere as the parent.
	shared_ptr<Link> sphere0 = make_shared<Link>(shapes->sphere);
	sphere0->scaleMesh(0.1f);
	parentLink = sphere0;

	// Generate some random bubbles around the center bubble.
	Vector2f bubbleRange(5.0f, 15.0f);
	Vector2f bubbleBox(-1.0f, 1.0f);
	int numBubbles = int(randRange(bubbleRange[0], bubbleRange[1]));
	for (int i = 0; i < numBubbles; i++) {
		shared_ptr<Link> sphere = make_shared<Link>(shapes->sphere);
		sphere->translateParent(Vector3f(randRangef(bubbleBox[0], bubbleBox[1]), 
			randRangef(bubbleBox[0], bubbleBox[1]), randRangef(bubbleBox[0], bubbleBox[1])));
		sphere->scaleMesh(0.1f);
		sphere0->addChild(sphere);
	}
}

Bubbles::~Bubbles() {
}

void Bubbles::draw(const shared_ptr<Program> prog, double t){
	// Randomize x, z position of bubbles reach the top.
	float height = speed * float(t) + yOffset;
	int quotient = int(height) / int(yRange(1) - yRange(0));
	if (quotient > count) {
		count = float(quotient);
		M->popMatrix();
		M->pushMatrix();
		M->translate(Vector3f(randRangef(xRange[0], xRange[1]), 0.0f, randRangef(zRange[0], zRange[1])));
	}

	// Move the bubbles vertically looping over the max y-range.
	height = fmod(height, yRange(1) - yRange(0)) + yRange(0);
	M->pushMatrix();
	M->translate(Vector3f(0.0f, height, 0.0f));
	parentLink->draw(prog, M);
	M->popMatrix();
}
