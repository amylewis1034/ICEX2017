#include "Node.hpp"
#include "Rectangle.hpp"
#include "ViewFrustrum.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GameObject.hpp"
#include "Components/Transform.hpp"
#include "Components/Mesh.hpp"
#include "Components/Camera.hpp"

#include <glm/gtx/string_cast.hpp>
#include <iostream>

/*  https://gamedevelopment.tutsplus.com/tutorials/
	quick-tip-use-QuadTrees-to-detect-likely-
	collisions-in-2d-space--gamedev-374

	http://www.txutxi.com/?p=584*/

Node::Node() {
	hasChildren = false;
	level = 0;
	bounds = new Rectangle(-500, 500, -500, 500);
}

Node::Node(int level, Rectangle *bounds) :
	level(level), bounds(bounds) {
		hasChildren = false;
	}

Node::~Node() {}

/* Adds a new game object to the tree */
void Node::addGameObject(GameObject *newObject) {
	if (hasChildren) {
		int index = getIndex(newObject);
		// std::cout << index << std::endl;
		if (index != -1) {
			children.at(index)->addGameObject(newObject);
			return;
		}
	}
	// std::cout << "TAG: " << newObject->getTag() << std::endl;

	// if (gameobjects.size() >= MAX_OBJECTS || level == 0) {
	if (gameobjects.size() + 1 > MAX_OBJECTS) {
		// std::cout << "Too many game objects" << std::endl;
		// split into 4 quadrants and recreate tree
		if (!hasChildren) {
			createChildren();
		}
		// add current Node's game objects to it's newly created children
		int i = 0;
		while (i < gameobjects.size()) {
			GameObject *temp = gameobjects.at(i);
			int index = getIndex(temp);
			if (index != -1) {
				gameobjects.erase(gameobjects.begin() + i);
				children.at(index)->addGameObject(temp);
				// std::cout << index << std::endl;
			}
			else {
				i++;
			}
		}
	}
	gameobjects.push_back(newObject);
}

/* Creates child Nodes with correct bounding boxes */
void Node::createChildren() {
	hasChildren = true;

	int width = bounds->getWidth();
	int height = bounds->getHeight();

	int subWidth = (int) (width / 2);
	int subHeight = (int) (height / 2);

	int x = (int) bounds->getLeftX();
	int z = (int) bounds->getInnerZ();

	Rectangle *upperLeft = new Rectangle(x, x + subWidth, z, z + subHeight);
	Rectangle *upperRight = new Rectangle(x + subWidth, x + width, z, z + subHeight);
	Rectangle *lowerLeft = new Rectangle(x, x + subWidth, z + subHeight, z + height);
	Rectangle *lowerRight = new Rectangle(x + subWidth, x + width, z + subHeight, z + height);


	children.push_back(new Node(level + 1, upperLeft));
	children.push_back(new Node(level + 1, upperRight));
	children.push_back(new Node(level + 1, lowerLeft));
	children.push_back(new Node(level + 1, lowerRight));

}

int Node::getIndex(GameObject *gameObject) {
	Rectangle *pRect = new Rectangle(gameObject);
	int index = -1;
	float horizontalMidPt = bounds->getLeftX() + (bounds->getWidth()/2);
	float vertMidPt = bounds->getInnerZ() + (bounds->getHeight() /2);

	bool topQuad = pRect->getInnerZ() < horizontalMidPt && pRect->getInnerZ() 
		+ pRect->getHeight() < horizontalMidPt;
	bool botQuad = pRect->getInnerZ() > horizontalMidPt;

	if (pRect->getLeftX() < vertMidPt && pRect->getLeftX() + pRect->getWidth() < vertMidPt) {
		//left
		if (topQuad) {
			index = 0;
		}
		else if (botQuad) {
			index = 2;
		}
	}
	else if (pRect->getLeftX() > vertMidPt) {
		//right
		if (topQuad) {
			index = 1;
		}
		else if (botQuad) {
			index = 3;
		}
	}
	return index;
}

std::vector<GameObject*> Node::getCollisions(GameObject *checkObj) {
	std::vector<GameObject*> collisions;
	int index = getIndex(checkObj);
	if (index != -1) {
		if (hasChildren) {
			std::vector<GameObject*> ret = children.at(index)->getCollisions(checkObj);
			collisions.insert(collisions.end(), ret.begin(), ret.end());
		}
	}

	Rectangle *rect = new Rectangle(checkObj);
	for (GameObject *g : gameobjects) {
		if (rect->intersect(g)) {
			collisions.push_back(g);
		}
	}
	return collisions;
}

void Node::removeGameObject(GameObject *checkObj) {
	std::vector<GameObject*> collisions;
	int index = getIndex(checkObj);
	if (index != -1) {
		if (hasChildren) {
			children.at(index)->removeGameObject(checkObj);
		}
	}

	Node *child = this;
	for (int i = 0; i < child->gameobjects.size(); i++) {
		if (child->gameobjects[i] == checkObj) {
			child->gameobjects.erase(child->gameobjects.begin() + i);
			return;
		}
	}
	return;
}

std::vector<GameObject*> Node::getRenderable(ViewFrustrum *frustrum) {
	std::vector<GameObject*> renderable;

	if (hasChildren) {
		for (Node *child : children) {
			if (intersect(frustrum, child->bounds)) {
				std::vector<GameObject*> ret = child->getRenderable(frustrum);
				renderable.insert(renderable.end(), ret.begin(), ret.end());
			}	
		}
	}

	for (GameObject *g : gameobjects) {
		if (intersect(frustrum, new Rectangle(g))) {
			renderable.push_back(g);
		}
	}
	return renderable;
}

bool Node::intersect(ViewFrustrum *frustrum, Rectangle *rect) {
	float rectX[2] = {rect->getLeftX(), rect->getRightX()};
	float rectZ[2] = {rect->getInnerZ(), rect->getOuterZ()};
	float rectY[2];
	// if (rect->isLTree()) {
		rectY[0] = rect->getLowerY();
		rectY[1] = rect->getUpperY();
	// }
	for (int i = 0; i < 6; i++) {
		glm::vec4 plane = frustrum->planes[i];
		int px = static_cast<int>(plane.x > 0.0f);
		int py = static_cast<int>(plane.y > 0.0f);
		int pz = static_cast<int>(plane.z > 0.0f);

		float dp;
		dp = (plane.x * rectX[px]) +
			(plane.y * rectY[py]) +	
			(plane.z * rectZ[pz]);

		// if (rect->isLTree()) {
		// 	dp = (plane.x * rectX[px]) +
		// 	(plane.y * rectY[py]) +	
		// 	(plane.z * rectZ[pz]);
		// }
		// else {
		// 	dp = (plane.x * rectX[px]) +
		// 	(plane.z * rectZ[pz]);
		// }

		if (dp + 1 < -plane.w) {
			// doesn't intersect with plane, cull
			return false;
		}
	}
	return true;
}

void Node::printNode() {
	std::string spacing;
	for (int i = 0; i < level; i++) {
		spacing += "   ";
	}
	std::cout << std::endl << spacing << "Level: " << level << std::endl;
	for (GameObject *g : gameobjects) {
		std::cout << spacing << "Quadrant: " << getIndex(g) << ", " << g->getTag() << ": " << g << std::endl;
	}
	if (hasChildren) {
		for (Node *child : children) {
			child->printNode();
		}
	}
}