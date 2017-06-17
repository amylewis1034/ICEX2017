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

#include "PRMInput.hpp"
#include "Transform.hpp"
#include <GameObject.hpp>

using namespace std;
// using namespace Eigen;

PRMInput::PRMInput(const std::string pathfile) :
    speed(10.0f),
    pathfile(pathfile),
    totalPathLen(12),
    splineNum(450 / totalPathLen)
    {}

PRMInput::PRMInput(float speed, const std::string pathfile, bool generatePath) :
	speed(speed),
    pathfile(pathfile),
    totalPathLen(12),
    splineNum(450 / totalPathLen),
    generatePath(generatePath)
	{}

PRMInput::~PRMInput() {}

void PRMInput::init() {
    transform = gameobject->getComponent<Transform>();
    assert(transform != nullptr);

    if (generatePath) {
        cout << "Generating path" << endl;
    }

    initCamPath();
}

void PRMInput::update(float dt) {
    static int curIter = 0;

    // float move_speed = dt * speed;
    // glm::vec3 delta = glm::vec3(0.0f);

    curIter = ((curIter + 1) % camPosVec.size());
    setCamPos6dof(camPosVec[curIter], camDirVec[curIter]);

    transform->setPosition(camPos);
    transform->setForward(camDir);

    // transform->translate(delta);
}

float PRMInput::getSpeed() const {
	return speed;
}

void PRMInput::setSpeed(float speed) {
	this->speed = speed;
}

void PRMInput::initCamPath() {
   string fileLocation;
   if (this->pathfile.length() > 0) {
      fileLocation = string(RESOURCE_PATH) + this->pathfile;
   }
   else {
      fileLocation = string(RESOURCE_PATH "path.txt");
   }
   ifstream pathFile(fileLocation);
   if (!pathFile.is_open()) {
      std::cerr << "Cannot open path input file" << std::endl;
      exit(1);
   }
   // Read path length
   pathFile >> pathLength;

   // Initalize temporary position & direction vectors from nodes in file
   vector<Eigen::Vector3f> tempPosVec;
   vector<Eigen::Vector3f> tempDirVec;

   float pX, pY, pZ, dX, dY, dZ;
   for (int i = 0; i < pathLength; i++) {
      pathFile >> pX >> pY >> pZ >> dX >> dY >> dZ;

      tempPosVec.push_back(Eigen::Vector3f(pX, pY, pZ));
      tempDirVec.push_back(Eigen::Vector3f(dX, dY, dZ));
   }
   // Push first and second node onto path again to create a complete spline
   tempPosVec.push_back(tempPosVec[0]);
   tempPosVec.push_back(tempPosVec[1]);
   tempDirVec.push_back(tempDirVec[0]);
   tempDirVec.push_back(tempDirVec[1]);

   // Print other statistics about path generation
   int roadMapSize;
   double avgWeight;
   double algTime;
   pathFile >> roadMapSize >> avgWeight >> algTime;
   cout << "pathLength: " << pathLength 
      << ", roadMapSize: " << roadMapSize 
      << ", avgWeight: " << avgWeight 
      << ", algTime: " << algTime << endl;

   // Spline variables, based on Sueda's animation assignment 1
   Eigen::MatrixXf Gpos(3,4); // 3 by 4 matrix
   Eigen::MatrixXf Gdir(3,4); // 3 by 4 matrix
   Eigen::Matrix4f B;      // 4 by 4 matrix
   Eigen::Vector4f uVec;   // 4 by 1 vector
   Eigen::Vector3f p;      // 3 by 1 vector
   B << 0, -1, 2, -1, 2 ,0, -5, 3, 0, 1, 4, -3, 0, 0, -1, 1;
   B = 0.5 * B;

   // Generate position and directions using Catmull-Rom splines
   for (int j = 0; j < pathLength; j++) {
      Gpos << tempPosVec[j % pathLength], 
         tempPosVec[(j+1) % pathLength], 
         tempPosVec[(j+2) % pathLength], 
         tempPosVec[(j+3) % pathLength];
      Gdir << tempDirVec[j % pathLength], 
         tempDirVec[(j+1) % pathLength], 
         tempDirVec[(j+2) % pathLength], 
         tempDirVec[(j+3) % pathLength];  
      for (float u = 0; u < 1; u += 1.0 / splineNum) {
         uVec << 1, u, u*u, u*u*u;
      
         auto pvec = Gpos * B * uVec;
         auto dvec = Gdir * B * uVec;

         camPosVec.push_back(glm::vec3(pvec[0], pvec[1], pvec[2]));
         camDirVec.push_back(glm::vec3(dvec[0], dvec[1], dvec[2]));       
      }
   }

   // Set camera to first position in path
   setCamPos6dof(camPosVec[0], camDirVec[0]);
}

void PRMInput::setCamPos6dof(const glm::vec3 pos, const glm::vec3 dir) {
    camPos = pos;
    camDir = dir;
}