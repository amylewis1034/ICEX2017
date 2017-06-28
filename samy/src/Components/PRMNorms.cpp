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

#include "PRMNorms.hpp"
#include "Transform.hpp"
#include <GameObject.hpp>

#include <PRM/PRMNode.h>
#include <PRM/PRMAlg.h>
#include <PRM/ImageProcessor.h>

#include "opencv2/highgui.hpp"

using namespace std;

extern GLFWwindow *window;

const double weightThreshNorm = 0.25;

PRMNorms::PRMNorms(int numNodes) :
    numNodes(numNodes)
    {}

PRMNorms::~PRMNorms() {}

void PRMNorms::init() {
    transform = gameobject->getComponent<Transform>();
    assert(transform != nullptr);

    genNorms = true;

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

    std::cout << "initialized PRMNorms" << std::endl;
}

void PRMNorms::update(float dt) {

}

void PRMNorms::postrender(const glm::mat4 &projection, const glm::mat4 &view) {
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
    
    double nodeWeight;
    if (genNorms) {
        nodeWeight = detectNormals(ocvImg);
    } 

    if (generatingRootNode && genNorms) {
        if (nodeWeight > bestRootWeight || bestRootNode == NULL) {
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
            if (genNorms && bestRootWeight > weightThreshNorm) {
                highWeightNodes.push_back(bestRootNode->getNdx());
            }
            if (genNorms) {
                curNode = generatePRMNode(numNodes);
            }
        }
    }
    else if (generatingRootNode) {
        curNode->setWeight(nodeWeight);
    }
    else {
        curNode->setWeight(nodeWeight);
        if (genNorms && nodeWeight > weightThreshNorm) {
            highWeightNodes.push_back(curNode->getNdx());
        }
        if (genNorms) {
            curNode = generatePRMNode(numNodes);
        }
    }
}

void PRMNorms::setCamPos6dof(const glm::vec3 pos, const glm::vec3 dir) {
    camPos = pos;
    camDir = dir;
}