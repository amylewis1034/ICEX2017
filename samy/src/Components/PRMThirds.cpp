#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include <cassert>

#include "PRMThirds.hpp"
#include "Transform.hpp"
#include <GameObject.hpp>

#include <PRM/PRMNode.h>
#include <PRM/PRMAlg.h>
#include <PRM/ImageProcessor.h>

#include "opencv2/highgui.hpp"

#include <World.hpp>

using namespace std;

extern GLFWwindow *window;
extern World *world;

const double weightThreshThirds = 0.04;

PRMThirds::PRMThirds(int numNodes) :
    numNodes(numNodes)
    {}

PRMThirds::~PRMThirds() {}

void PRMThirds::init() {
    genThirds = true;

    transform = gameobject->getComponent<Transform>();
    // collider = gameobject->getComponent<Collider>();

    assert(transform != nullptr);
    // assert(collider != nullptr);

    GameObject *target = world->getGameObjectByTag("manoel");
    assert(target != nullptr);
    Transform *targetTransform = target->getComponent<Transform>();
    Collider *targetCollider = target->getComponent<Collider>();

    assert(targetTransform != nullptr);
    assert(targetCollider != nullptr);

    PRMNode::setCenterOfWorld(targetTransform->getPosition());
    PRMNode::setLowerLeftOfBB(targetCollider->getMin());
    PRMNode::setUpperRightOfBB(targetCollider->getMax());

    curNode = generateRootPRMNode(numNodes);
    // bestRootWeight = genThirds ? 1 : 0;
    bestRootWeight = 1;

    glfwGetFramebufferSize(window, &actualWidth, &actualHeight);

    glGenFramebuffers(1, &framebuffer);
    glGenTextures(1, &renderTexture);
    glGenRenderbuffers(1, &depthRenderBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glBindTexture(GL_TEXTURE_2D, depthRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, actualWidth, actualHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderTexture, 0);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, actualWidth, actualHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);

    // GLuint texture;
	// glGenTextures(1, &texture);

	// glBindTexture(GL_TEXTURE_2D, texture);
	// glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, actualWidth, actualHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	// glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture, 0);

    // GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    // glDrawBuffers(1, DrawBuffers);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "framebuffer error" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    std::cout << "initialized PRMThirds" << std::endl;
}

void PRMThirds::update(float dt) {

}

void PRMThirds::postrender(const glm::mat4 &projection, const glm::mat4 &view) {
    // Set the camera position & direction based on current node
    setCamPos6dof(curNode->getPosition(), curNode->getDirection());

    transform->setPosition(camPos);
    transform->setForward(camDir);
    
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
    glBlitFramebuffer(
        0, 0, actualWidth, actualHeight,
        0, 0, actualWidth, actualHeight,
        GL_COLOR_BUFFER_BIT, GL_NEAREST
    );
     glBindFramebuffer(GL_FRAMEBUFFER, 0);
   
    cv::Mat ocvImg = ocvImgFromGlTex(renderTexture);
    double nodeWeight = std::abs(detectThirds(ocvImg) - 0.66666666);

    if (generatingRootNode && genThirds) {
        if (nodeWeight < bestRootWeight) {
            bestRootNode = curNode;
            bestRootWeight = nodeWeight;
        }
        if (rootIter < 200) {
            curNode = generateRootPRMNode(numNodes);
            rootIter++;
        }
        else {
            setRootPRMNode(bestRootNode);
            generatingRootNode = false;
            bestRootNode->setWeight(bestRootWeight);
            if (genThirds && bestRootWeight < weightThreshThirds) {
                highWeightNodes.push_back(bestRootNode->getNdx());
            }
            if (genThirds) {
                curNode = generatePRMNode(numNodes);
            }
        }
    }
    else if (generatingRootNode) {
        curNode->setWeight(nodeWeight);
    }
    else {
        curNode->setWeight(nodeWeight);
        if (genThirds && nodeWeight < weightThreshThirds) {
            highWeightNodes.push_back(curNode->getNdx());
        }
        if (genThirds) {
            curNode = generatePRMNode(numNodes);
        }
    }
}

void PRMThirds::setCamPos6dof(const glm::vec3 pos, const glm::vec3 dir) {
    camPos = pos;
    camDir = dir;
}