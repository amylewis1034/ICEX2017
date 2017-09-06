#pragma once
#ifndef _PRMNODE_H_
#define _PRMNODE_H_

#include <string>
#include <math.h>
#include <vector>

#include <glm/glm.hpp>

const int minHeight = 2, maxHeight = 24;

class PRMNode {

public:
	PRMNode(int numNodes);
	PRMNode(int numNodes, PRMNode *withinDelta);

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
	glm::vec3 calcFreeDirection(glm::vec3 pos, float theta, int pathLength);
	glm::vec3 calcFreePosition(float height, int pathLength, int numNodes);
	float getCamTheta() { return camTheta; }
	float getCamPhi() { return camPhi; }
	static void setCenterOfWorld(glm::vec3 centerPos);
	static void setUpperRightOfBB(glm::vec3 upperRightPos);
	static void setLowerLeftOfBB(glm::vec3 lowerLeftPos);
	static glm::vec3 getCenterOfWorld();
	static glm::vec3 getUpperRightOfBB();
	static glm::vec3 getLowerLeftOfBB();

	static glm::vec3 centerOfWorld;
	static glm::vec3 upperRightOfBB;
	static glm::vec3 lowerLeftOfBB;	
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
