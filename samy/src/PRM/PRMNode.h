#pragma once
#ifndef _PRMNODE_H_
#define _PRMNODE_H_

#include <string>
#include <math.h>
#include <vector>

#include <glm/glm.hpp>

const int minHeight = 0, maxHeight = 14;

class PRMNode {

public:
	PRMNode();
	PRMNode(PRMNode *withinDelta);

	glm::vec3 getPosition() {return position;}
	glm::vec3 getDirection() {return direction;}
	float getHeight() {return position[1];}
	float getTheta() {return direction[1];}
	PRMNode *getParent(){return parent;}
	int getPathLength(){return pathLength;}
	int getNdx() {return ndx;}
	void setNdx(int index) {ndx = index;}
	double getWeight() {return weight;}
	void setWeight(double nodeWeight) {weight = nodeWeight;}
	glm::vec3 calcFreeDirection(float theta, int pathLength);
	glm::vec3 calcFreePosition(float height, int pathLength);
	float getCamTheta() { return camTheta; }
	float getCamPhi() { return camPhi; }

	static glm::vec3 centerWorld;
	static glm::vec3 constVelMults;

private:
	glm::vec3 position;
	glm::vec3 direction;
	PRMNode *parent;
	int pathLength;
	int ndx;
	double weight;
	float camTheta;
	float camPhi;
	float robotTheta;
};

#endif
