#include "PRMNode.h"
#include "Utilities.h"
#include <stddef.h>
#include <iostream>

const float heightMaxDelta = 2.5;
const float thetaMaxDelta = M_PI / 24;
const float initalTheta = -M_PI_4 / 2;
const float posDelta = (2 * M_PI) / totalPathLen;
const float offset = M_PI;
const double radius = 26;

// Calculate the position of the node based on how far it is along circle
Eigen::Vector3f calcPosition(float height, int pathLength) {
   double posTheta = pathLength * posDelta;

   double curRadius = randRangef(radius - 1, radius + 1);
   return Eigen::Vector3f(
   	1.0 * curRadius * cos(posTheta) + 6, 
   	height, 
   	0.8 * curRadius * sin(posTheta) - 1);
}

// Calculate the direction the node should point to look at center of circle 
// based how far it is along circle
Eigen::Vector3f calcDirection(float theta, int pathLength) {
	double dirTheta = pathLength * posDelta;

	return Eigen::Vector3f(1.0 * cos(offset + dirTheta), theta, 0.8 * -sin(dirTheta));
}

PRMNode::PRMNode() {
	parent = NULL;
	pathLength = 1;
	weight = 0;

	// Strange bug where first time rand() is called, we always get the same 
	// value so we call it once before we actully use it
	randRangef(4.0, 16.0); 
	position = calcPosition(randRangef(4.0, 12.0), pathLength - 1);
	direction = calcDirection(
		randRangef(
			initalTheta - 3 * thetaMaxDelta, 
			initalTheta + 3 * thetaMaxDelta), 
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

	position = calcPosition(height, pathLength - 1);
	direction = calcDirection(
		randRangef(parentT - thetaMaxDelta, parentT + thetaMaxDelta), 
		pathLength - 1);
}