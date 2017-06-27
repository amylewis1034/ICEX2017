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

    genThirds = false;

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

    // Get current frame buffer size.
    // Important for retina displays!
    glfwGetFramebufferSize(window, &actualWidth, &actualHeight);
    glViewport(0, 0, actualWidth, actualHeight);

    // Write to the framebuffer so we can transfer the image to OpenCV
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer); // This goes with writeToTexture call

    // Clear framebuffer.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    float aspect = actualWidth/(float)actualHeight;

    cv::Mat ocvImg = ocvImgFromGlTex(renderTexture);
    double nodeWeight = detectNormals(ocvImg);

    if (generatingRootNode) {
        if (nodeWeight > bestRootWeight) {
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
            curNode = generatePRMNode(numNodes);
        }
    }
    else {
        curNode->setWeight(nodeWeight);
        if (genNorms && nodeWeight > weightThreshNorm) {
            highWeightNodes.push_back(curNode->getNdx());
        }
        curNode = generatePRMNode(numNodes);
    }
}

void PRMNorms::setCamPos6dof(const glm::vec3 pos, const glm::vec3 dir) {
    camPos = pos;
    camDir = dir;
}