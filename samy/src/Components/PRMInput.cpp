#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include <cassert>

#include "PRMInput.hpp"
#include "Transform.hpp"
#include <Input/Keyboard.hpp>
#include <GameObject.hpp>

using namespace std;

PRMInput::PRMInput(const std::string pathfile) :
    speed(10.0f),
    pathfile(pathfile),
    splineNum(450)
    {}

PRMInput::PRMInput(float speed, const std::string pathfile) :
	speed(speed),
    pathfile(pathfile),
    splineNum(450)
	{}

PRMInput::~PRMInput() {}

void PRMInput::init() {
    transform = gameobject->getComponent<Transform>();
    assert(transform != nullptr);

    simpleShader.linkProgram(SHADER_PATH "simple.vert", SHADER_PATH "simple.frag");

    initCamPath();

    pathsVertArrayObj.bind();
    glEnableVertexAttribArray(0);
    pathsVertBufObj.loadData(GL_ARRAY_BUFFER, camPosVec.size() * sizeof(glm::vec3), camPosVec.data(), GL_STATIC_DRAW);
    pathsVertArrayObj.addAttribute(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *)0);
    
    pathsVertArrayObj.unbind();
    pathsVertBufObj.unbind();
}

void PRMInput::update(float dt) {
    static int curIter = 0;

    curIter = ((curIter + 1) % camPosVec.size());
    setCamPos6dof(camPosVec[curIter], camDirVec[curIter]);

    transform->setPosition(camPos);
    transform->setForward(camDir);
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
   cout << "Opening path " << fileLocation << endl;
   ifstream pathFile(fileLocation);
   if (!pathFile.is_open()) {
      std::cerr << "Cannot open path input file" << std::endl;
      exit(1);
   }
   // Read path length
   pathFile >> pathLength;

   // Initalize temporary position & direction vectors from nodes in file
   vector<glm::vec3> tempPosVec;
   vector<glm::vec3> tempDirVec;

   float pX, pY, pZ, dX, dY, dZ;
   for (int i = 0; i < pathLength; i++) {
      pathFile >> pX >> pY >> pZ >> dX >> dY >> dZ;

      tempPosVec.push_back(glm::vec3(pX, pY, pZ));
      tempDirVec.push_back(glm::vec3(dX, dY, dZ));
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
   glm::mat4x3 Gpos; // 3 by 4 matrix
   glm::mat4x3 Gdir; // 3 by 4 matrix
   glm::mat4 B = glm::mat4(0.0f, 1.0f, 0.0f, 0.0f, -0.5f, 0.0f, 0.5f, 0.0f, 1.0f, -2.5, 2.0f, -0.5f, -0.5f, 1.5f, -1.5f, 0.5f); // 4 by 4 matrix  
   glm::vec4 uVec;   // 4 by 1 vector
   glm::vec3 p;      // 3 by 1 vector]

   // Generate position and directions using Catmull-Rom splines
   for (int j = 0; j < pathLength; j++) {
      Gpos[0] = tempPosVec[j % pathLength]; 
      Gpos[1] = tempPosVec[(j+1) % pathLength];
      Gpos[2] = tempPosVec[(j+2) % pathLength];
      Gpos[3] = tempPosVec[(j+3) % pathLength];
      Gdir[0] = tempDirVec[j % pathLength]; 
      Gdir[1] = tempDirVec[(j+1) % pathLength];
      Gdir[2] = tempDirVec[(j+2) % pathLength]; 
      Gdir[3] = tempDirVec[(j+3) % pathLength];  
      for (float u = 0; u < 1; u += (float)pathLength / splineNum) {
         uVec = glm::vec4(1, u, u*u, u*u*u);
      
         auto pvec = Gpos * B * uVec;
         auto dvec = Gdir * B * uVec;

         camPosVec.push_back(glm::vec3(pvec[0], pvec[1], pvec[2]));
         camDirVec.push_back(glm::vec3(dvec[0], dvec[1], dvec[2]));       
      }
   }

   // Set camera to first position in path
   setCamPos6dof(camPosVec[0], camDirVec[0]);
}

void PRMInput::postrender(const glm::mat4 &projection, const glm::mat4 &view) {
    if (Keyboard::getKeyToggle(GLFW_KEY_L)) {
        simpleShader.bind();
        static glm::mat4 model;
        glUniformMatrix4fv(simpleShader.uniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(simpleShader.uniformLocation("view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(simpleShader.uniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));

        pathsVertArrayObj.bind();
        glLineWidth(4);
        glDrawArrays(GL_LINE_STRIP, 0, camPosVec.size());
        pathsVertArrayObj.unbind();

        simpleShader.unbind();
    }
}

void PRMInput::setCamPos6dof(const glm::vec3 pos, const glm::vec3 dir) {
    camPos = pos;
    camDir = dir;
}
