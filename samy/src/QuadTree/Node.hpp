#ifndef NODE_HPP
#define NODE_HPP

#include "GameObject.hpp"
#include "Rectangle.hpp"
#include "ViewFrustrum.hpp"

/* references: 
    https://gamedevelopment.tutsplus.com
    /tutorials/quick-tip-use-QuadTrees-to-
    detect-likely-collisions-in-2d-space--gamedev-374

    https://www.gamedev.net/resources/_/technical/
    graphics-programming-and-theory/QuadTrees-r1303

*/
#define MAX_OBJECTS 500

class Node {
public:
    Node();
    Node(int level, Rectangle *bounds);
    ~Node();

    void addGameObject(GameObject *newObject);
    void createChildren();
    int getIndex(GameObject *newObject);
    void removeGameObject(GameObject *checkObj);
    void printNode();
    std::vector<GameObject*> getCollisions(GameObject *checkObj);
    std::vector<GameObject*> getRenderable(ViewFrustrum *frustrum);
    bool intersect(ViewFrustrum *frustrum, Rectangle *rect);
    float DistToPlane(float A, float B, float C, float D, glm::vec3 point);
    float distToPlane(glm::vec4 plane, float x, float z);

private:
    int level;
    Rectangle *bounds;
    std::vector<GameObject *> gameobjects;
    std::vector<Node *> children;
    bool hasChildren;
};

#endif