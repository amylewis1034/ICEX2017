#ifndef RECTANGLE_HPP
#define RECTANGLE_HPP

#include "GameObject.hpp"
#include <glm/glm.hpp>

class Rectangle {
public:
    Rectangle();
    Rectangle(GameObject *gameObject);
    // Rectangle(glm::vec3 topL, glm::vec3 topR, glm::vec3 botL, glm::vec3 botR);
    Rectangle(float leftX, float rightX, float innerZ, float outerZ);
    ~Rectangle();

    void setRect(GameObject *gameObject);

    float getLeftX();
    float getRightX();
    float getInnerZ();
    float getOuterZ();
    float getWidth();
    float getHeight();

    bool intersect(GameObject *gameobject);

    void printRect();

    bool isLTree();

    float getLowerY();
    float getUpperY();
    void itIsALTree(GameObject *g);

private:
    // glm::vec3 topL;
    // glm::vec3 topR;
    // glm::vec3 botL;
    // glm::vec3 botR;

    float leftX, rightX;
    float outerZ, innerZ;
    float lowerPt, upperPt;

    float width, height;

    bool lTree;

    std::string tag;
};

#endif