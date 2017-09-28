#include "PRMNode.h"
#include "Utilities.h"
#include <stddef.h>
#include <iostream>
#include <World.hpp>

extern World *world;

const float heightMaxDelta = 2.5;
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
   double posTheta = pathLength * ((2 * M_PI) / numNodes);
   double curRadius = randRangef(radius - 1, radius + 1);
   return glm::vec3(
			PRMNode::constVelMults[0] * curRadius * cos(posTheta) + PRMNode::getCenterOfWorld()[0], 
			PRMNode::constVelMults[1] * height + PRMNode::getCenterOfWorld()[1],
			PRMNode::constVelMults[2] * curRadius * sin(posTheta) + PRMNode::getCenterOfWorld()[2]);
}

// Calculate the direction the node should point to look at center of circle 
// based how far it is along circle
glm::vec3 calcDirection(float theta, int pathLength, int numNodes) {
	double dirTheta = pathLength * ((2 * M_PI) / numNodes);

	return glm::vec3(PRMNode::constVelMults[0] * cos(offset + dirTheta), PRMNode::constVelMults[1] * dirTheta, PRMNode::constVelMults[2] * -sin(dirTheta));
}

glm::vec3 PRMNode::calcFreePosition(float height, int pathLength, int numNodes) {
	double posTheta = (pathLength) * ((2 * M_PI) / numNodes );
	double curRadius = randRangef(radius - 1, radius + 1);
	float posX, posY, posZ;
	glm::vec3 output;

	if (getParent() == NULL) {
		output = glm::vec3(
			posX = PRMNode::getCenterOfWorld()[0] + curRadius, 
			posY = PRMNode::getCenterOfWorld()[1] + height,
			posZ = PRMNode::getCenterOfWorld()[2] + curRadius);
	}
	else {
		posX = getParent()->getPosition()[0];
		posY = getParent()->getPosition()[1];
		posZ = getParent()->getPosition()[2];
	

	float distance = -10;
    float randWidth = randRangef(-3, 3);
	float randHeight = randRangef(-2, 2);
	
//	while (distance < .5 * curRadius || distance > 4 * curRadius) {
		output = glm::vec3(
			// posX * robotVelocity * cos(posTheta), //can be -9 to 9 if at lowest with parent position
			// posY + height, //check to see if going through ground
			// posZ * robotVelocity * sin(posTheta)); //can be -9 to 9 if at lowest with parent position
			posX + curRadius * tan(M_PI / numNodes) * cos(posTheta + M_PI / 2) + randWidth,
			posY + randHeight,
			posZ + curRadius * tan(M_PI / numNodes) * sin(posTheta + M_PI / 2) + randWidth);
		distance = glm::sqrt((posZ - output.z) * (posZ - output.z) + (posY - output.y) * (posY - output.y) + (posX - output.x) * (posX - output.x));
	//}
	}

	return output;
}

glm::vec3 PRMNode::calcFreeDirection(glm::vec3 pos, float theta, int pathLength, int numNodes) {
	float randTheta = randRangef(M_PI / (3.0f * float(numNodes)), M_PI / float(numNodes));
	float randPhi = randRangef(M_PI / (3.0f * float(numNodes)), M_PI / float(numNodes));

	float curTheta, curPhi;
	if (getParent() == NULL) {
		curTheta = M_PI + randTheta;
		curPhi = randPhi;
	}
	else {
		curTheta = getParent()->getCamTheta() + randTheta;
		curPhi = getParent()->getCamPhi() + randPhi;
	}

	camTheta = curTheta;
	camPhi = curPhi;

	float camX = cos(curPhi) * cos(curTheta);
	float camY = sin(curPhi);
	float camZ = cos(curPhi) * cos(M_PI/2 - curTheta);

	glm::vec3 cameraPos = glm::vec3( camX, camY, camZ );
	glm::vec3 cameraTarget = glm::vec3(PRMNode::getCenterOfWorld()[0] + radius / 2, PRMNode::getCenterOfWorld()[1], PRMNode::getCenterOfWorld()[2] + radius / 2);
	glm::vec3 cameraDirection = glm::normalize(cameraTarget - pos);

	return cameraDirection;
}

PRMNode::PRMNode(int numNodes) {
	float thetaMaxDelta = M_PI / (2 * numNodes);
	parent = NULL;
	pathLength = 1;
	weight = 0;
	//Maybe start towards to wreck
	camPhi = 0;
	camTheta = 2 * M_PI / numNodes;
	robotTheta = randRangef(0.0f, M_PI);

	// Strange bug where first time rand() is called, we always get the same 
	// value so we call it once before we actully use it
	randRangef(4.0, 16.0); 
	if (world->getRenderSetting().isKatie == true) {
		position = calcPosition(randRangef(4.0, 12.0), pathLength - 1, numNodes);
		direction = calcDirection(
			randRangef(
				initalTheta - 3 * thetaMaxDelta, 
				initalTheta + 3 * thetaMaxDelta), //random pitch
			pathLength - 1,
			numNodes);
	}
	else {
		position = calcFreePosition(randRangef(10.0, 12.0), pathLength - 1, numNodes);
		direction = calcFreeDirection(position,
			randRangef(
				initalTheta - 3 * thetaMaxDelta, 
				initalTheta + 3 * thetaMaxDelta), //random pitch
			pathLength - 1,
			numNodes);
	}
}

PRMNode::PRMNode(int numNodes, PRMNode *withinDelta) {
	float thetaMaxDelta = M_PI / (2 * numNodes);
	weight = 0;
	parent = withinDelta;
	pathLength = withinDelta->getPathLength() + 1;

	// Get the height and theta of the node we're expanding off of
	float parentH = withinDelta->getHeight();
	float parentT = withinDelta->getTheta();

	double height = -1;
	// Get a height above the seafloor and below our max height
	height = glm::clamp(randRangef(parentH - heightMaxDelta, parentH + heightMaxDelta), minHeight + PRMNode::getCenterOfWorld()[1], maxHeight + PRMNode::getCenterOfWorld()[1]);

	if (world->getRenderSetting().isKatie == true) {
		position = calcPosition(height, pathLength - 1, numNodes);
		direction = calcDirection(
			randRangef(
				parentT - thetaMaxDelta, 
				parentT + thetaMaxDelta), 
			pathLength - 1,
			numNodes);
	}
	else {
		position = calcFreePosition(height, pathLength - 1, numNodes);
		direction = calcFreeDirection(position,
			randRangef(parentT - thetaMaxDelta, parentT + thetaMaxDelta), 
			pathLength - 1,
			numNodes);
	}
}

void PRMNode::setCenterOfWorld(glm::vec3 centerPos) {
	PRMNode::centerOfWorld = centerPos - glm::vec3(radius / 2, 0, radius /2);
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
