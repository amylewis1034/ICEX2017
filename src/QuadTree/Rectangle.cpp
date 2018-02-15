#include "Rectangle.hpp"
#include "GameObject.hpp"

#include "Components/Mesh.hpp"
#include "Components/Collider.hpp"
#include "Components/Transform.hpp"
#include "Components/Camera.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include <iostream>
#include <cmath>

Rectangle::Rectangle() {}
Rectangle::Rectangle(GameObject *gameObject) {
	lTree = false;
	tag = gameObject->getTag();
	setRect(gameObject);
}
Rectangle::Rectangle(float leftX, float rightX, float innerZ, float outerZ) :
	leftX(leftX), rightX(rightX), innerZ(innerZ), outerZ(outerZ) {
		lTree = false;
		width = rightX - leftX;
		height = outerZ - innerZ;
	}
Rectangle::~Rectangle() {}

void Rectangle::setRect(GameObject *gameObject) {
	Transform *t = gameObject->getComponent<Transform>();

	leftX = t->getPosition().x - (t->getScale().x/2);
	rightX = t->getPosition().x + (t->getScale().x/2);

	innerZ = t->getPosition().z - (t->getScale().z/2);
	outerZ = t->getPosition().z + (t->getScale().z/2);

	lowerPt = t->getPosition().y - (t->getScale().y/2);
	upperPt = t->getPosition().y + (t->getScale().y/2);

	width = rightX - leftX;
	height = outerZ - innerZ;


	// if (gameObject->hasComponent<Camera>()) {
	// 	printf(" camera");
	// }

	if (gameObject->getTag() == "orb") {
		float buff = 0.5f;
		leftX = t->getPosition().x - buff;
		rightX = t->getPosition().x + buff;
		innerZ = t->getPosition().z - buff;
		outerZ = t->getPosition().z + buff;
	}
    // if (gameObject->hasComponent<Mesh>()) {
    // 	printf(" mesh");
    // }
    // if (gameObject->hasComponent<Collider>()) {
    // 	printf(" collider");
    // }
	upperPt = 15;
}

void Rectangle::itIsALTree(GameObject *g) {
	Transform *t = g->getComponent<Transform>();
	lTree = true;
	float newlowerPt = t->getPosition().y - (t->getScale().y/2);
	float newupperPt = t->getPosition().y + (t->getScale().y/2);

	lowerPt = glm::min(newlowerPt, lowerPt);
	upperPt = glm::max(newupperPt, upperPt);
}

bool Rectangle::isLTree() {
	return lTree;
}

float Rectangle::getLeftX() {
	return leftX;
}

float Rectangle::getRightX() {
	return rightX;
}

float Rectangle::getInnerZ() {
	return innerZ;
}

float Rectangle::getOuterZ() {
	return outerZ;
}

float Rectangle::getLowerY() {
	return lowerPt;
}

float Rectangle::getUpperY() {
	return upperPt;
}

float Rectangle::getWidth() {
	return width;
}

float Rectangle::getHeight() {
	return height;
}

bool Rectangle::intersect(GameObject *gameobject) {
	Rectangle *test = new Rectangle(gameobject);
	if (leftX < test->getRightX() && rightX > test->getLeftX() 
		&& innerZ < test->getOuterZ() && outerZ > test->getInnerZ()) {
		return true;
	}
	else {
		return false;
	}
} 

void Rectangle::printRect() {
	std::cout << tag << std::endl;
	std::cout << "Width: " << width << std::endl;
	std::cout << "Height: " << height << std::endl;
	std::cout << "leftX: " << leftX << std::endl;
	std::cout << "rightX: " << rightX << std::endl;
	std::cout << "innerZ: " << innerZ << std::endl;
	std::cout << "outerZ: " << outerZ << std::endl;
	std::cout << "lowerPt: " << lowerPt << std::endl;
	std::cout << "upperPt: " << upperPt<< std::endl;
	std::cout << std::endl;
}