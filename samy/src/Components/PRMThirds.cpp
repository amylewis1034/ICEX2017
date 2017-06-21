#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

#include <Eigen/Dense>

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

using namespace std;
// using namespace Eigen;

extern GLFWwindow *window;

const double weightThreshThirds = 0.04;

PRMThirds::PRMThirds() {}

PRMThirds::~PRMThirds() {}

void PRMThirds::init() {
    transform = gameobject->getComponent<Transform>();
    assert(transform != nullptr);

    genThirds = true;

    curNode = generateRootPRMNode();
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

void PRMThirds::postrender() {
    Eigen::Vector3f p, d;
    p = curNode->getPosition();
    d = curNode->getDirection();
    setCamPos6dof(
        glm::vec3(p[0], p[1], p[2]),
        glm::vec3(d[0], d[1], d[2])
    );

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
   
    
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
    // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    // glBlitFramebuffer(
    //     0, 0, actualWidth, actualHeight,
    //     0, 0, actualWidth, actualHeight,
    //     GL_COLOR_BUFFER_BIT, GL_NEAREST
    // );
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);

    cv::Mat ocvImg = ocvImgFromGlTex(renderTexture);
    // cv::namedWindow("TESTING");
    // cv::imshow("TESTING", ocvImg);

    double nodeWeight = std::abs(detectThirds(ocvImg) - 0.66666666);
    if (generatingRootNode && genThirds) {
        if (nodeWeight < bestRootWeight) {
            bestRootNode = curNode;
            bestRootWeight = nodeWeight;
        }
        if (rootIter < 200) {
            curNode = generateRootPRMNode();
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
                curNode = generatePRMNode();
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
            curNode = generatePRMNode();
        }
    }
}

void PRMThirds::setCamPos6dof(const glm::vec3 pos, const glm::vec3 dir) {
    camPos = pos;
    camDir = dir;
}