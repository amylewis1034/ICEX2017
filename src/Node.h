#pragma once
#ifndef _NODE_H_
#define _NODE_H_

#include <string>
#include <math.h>
#include <vector>

#define EIGEN_DONT_ALIGN_STATICALLY
#include <Eigen/Dense>

const int totalPathLen = 12;
const int minHeight = 0, maxHeight = 14;

class Node {

public:
	Node();
	Node(Node *withinDelta);

	Eigen::Vector3f getPosition() {return position;}
	Eigen::Vector3f getDirection() {return direction;}
	float getHeight() {return position[1];}
	float getTheta() {return direction[1];}
	Node *getParent(){return parent;}
	int getPathLength(){return pathLength;}
	int getNdx() {return ndx;}
	void setNdx(int index) {ndx = index;}
	double getWeight() {return weight;}
	void setWeight(double nodeWeight) {weight = nodeWeight;}

private:
	Eigen::Vector3f position;
	Eigen::Vector3f direction;
	Node *parent;
	int pathLength;
	int ndx;
	double weight;
};

#endif