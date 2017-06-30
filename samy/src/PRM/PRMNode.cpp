#include "PRMNode.h"
#include "Utilities.h"
#include <stddef.h>
#include <iostream>

const float heightMaxDelta = 2.5;
const float thetaMaxDelta = M_PI / 24;
const float initalTheta = -M_PI_4 / 2;
const float offset = M_PI;
const double radius = 26;
const float robotVelocity = 1.632f;
glm::vec3 PRMNode::centerOfWorld;
glm::vec3 PRMNode::upperRightOfBB;
glm::vec3 PRMNode::lowerLeftOfBB;	
glm::vec3 PRMNode::constVelMults = glm::vec3(1.0, 1.0, 0.8);

// Calculate the position of the node based on how far it is along circle
glm::vec3 calcPosition(float height, int pathLength, int numNodes) {
   double posTheta = pathLength * (2 * M_PI) / 20;

   double curRadius = randRangef(radius - 1, radius + 1);
   return glm::vec3(
   	PRMNode::constVelMults[0] * curRadius * cos(posTheta) + PRMNode::getCenterOfWorld()[0], 
   	PRMNode::constVelMults[1] * height + PRMNode::getCenterOfWorld()[1], 
   	PRMNode::constVelMults[2] * curRadius * sin(posTheta) + PRMNode::getCenterOfWorld()[2]);
}

glm::vec3 PRMNode::calcFreePosition(float height, int pathLength) {
   double posTheta = pathLength * (2 * M_PI) / 12;
   float posX, posY, posZ;

   if (getParent() == NULL) {
	   posX = PRMNode::getCenterOfWorld()[0]; 
	   posY = PRMNode::getCenterOfWorld()[1];
	   posZ = PRMNode::getCenterOfWorld()[2];
   }
   else {
	posX = getParent()->getPosition()[0];
	posY = getParent()->getPosition()[1];
	posZ = getParent()->getPosition()[2];
   }

   //LOOOPPPPP
   double curRadius = randRangef(radius - 1, radius + 1);
   return glm::vec3(
   	posX + robotVelocity * cos(getCamTheta()), 
   	posY + randRangef(-1, 1), //check to see if going through ground 
   	posZ + robotVelocity * sin(getCamTheta()));
}

// Calculate the direction the node should point to look at center of circle 
// based how far it is along circle
glm::vec3 calcDirection(float theta, int pathLength) {
	double dirTheta = pathLength * (2 * M_PI) / 20;

	return glm::vec3(1.0 * cos(offset + dirTheta), theta, 0.8 * -sin(dirTheta));
}

glm::vec3 PRMNode::calcFreeDirection(float theta, int pathLength) {
	float randTheta = randRangef(0.0872665f, 0.261799f);
	float randPhi = randRangef(0.0872665f, 0.261799f);

	float curTheta, curPhi;
	if (getParent() == NULL) {
		curTheta = M_PI + randTheta;
		curPhi = randPhi;
	}
	else {
		curTheta = getParent()->getCamTheta() + randTheta;
		curPhi = getParent()->getCamPhi() + randPhi;
	}

	float camX = cos(curPhi) * cos(curTheta);
	float camY = sin(curPhi);
	float camZ = cos(curPhi) * cos(M_PI/2 - curTheta);

	return glm::vec3( camX, camY, camZ );
}

PRMNode::PRMNode() {
	parent = NULL;
	pathLength = 1;
	weight = 0;
	//Maybe start towards to wreck
	camPhi = 0;
	camTheta = M_PI;
	robotTheta = randRangef(0.0f, M_PI);

	// Strange bug where first time rand() is called, we always get the same 
	// value so we call it once before we actully use it
	randRangef(4.0, 16.0); 
	position = calcFreePosition(randRangef(4.0, 12.0), pathLength - 1);
	direction = calcFreeDirection(
		randRangef(
			initalTheta - 3 * thetaMaxDelta, 
			initalTheta + 3 * thetaMaxDelta), //random pitch
		pathLength - 1);
}

PRMNode::PRMNode(PRMNode *withinDelta) {
	weight = 0;
	parent = withinDelta;
	pathLength = withinDelta->getPathLength() + 1;

	// Get the height and theta of the node we're expanding off of
	float parentH = withinDelta->getHeight();
	float parentT = withinDelta->getTheta();

	double height = -1;
	// Get a height above the seafloor and below our max height
	while (height < minHeight || height > maxHeight) {
		height = randRangef(parentH - heightMaxDelta, parentH + heightMaxDelta);
	}

	position = calcFreePosition(height, pathLength - 1);
	direction = calcFreeDirection(
		randRangef(parentT - thetaMaxDelta, parentT + thetaMaxDelta), 
		pathLength - 1);
}

void PRMNode::setCenterOfWorld(glm::vec3 centerPos) {
	PRMNode::centerOfWorld = centerPos;
}

void PRMNode::setUpperRightOfBB(glm::vec3 upperRightPos) {
	PRMNode::upperRightOfBB = upperRightPos;
}

void PRMNode::setLowerLeftOfBB(glm::vec3 lowerLeftPos) {
	PRMNode::lowerLeftOfBB = lowerLeftPos;
}

glm::vec3 PRMNode::getCenterOfWorld() {
	return PRMNode::centerOfWorld;
}

glm::vec3 PRMNode::getUpperRightOfBB() {
	return PRMNode::upperRightOfBB;
}

glm::vec3 PRMNode::getLowerLeftOfBB() {
	return PRMNode::lowerLeftOfBB;
}