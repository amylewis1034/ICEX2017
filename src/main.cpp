/*  Katie Davis' 2017 master's thesis

ICEX 2016 shipwreck environment

Based on K. Reimer 471 final project
 */
#include <iostream>
#include <stdio.h>
#include <string>
#include <fstream>
#include <unistd.h>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "GLSL.h"
#include "Program.h"
#include "Texture.h"
#include "Bubbles.h"
#include "Utilities.h"
// #include "Image.h"
#include "ImageProcessor.h"
#include "PRMAlg.h"

using namespace std;
using namespace Eigen;

bool keyToggles[256] = {false}; // only for English keyboards!

GLFWwindow *window; // Main application window
string RESOURCE_DIR = "resources/"; // Where the resources are loaded from

// GLSL programs
shared_ptr<Program> phongProg; // simple phong lighting shader
shared_ptr<Program> fadePhongProg;  // Fading objs with phong lighting
shared_ptr<Program> solidColorProg;  // draws objs in a solid color
shared_ptr<Program> fadeTexPhongProg;  // Fading, textured objs with phong lighting
shared_ptr<Program> fadeWavePhongProg;  // Fading, waving objs with phong lighting
shared_ptr<Program> tex_prog;  // Render the framebuffer texture to the screen
shared_ptr<Program> normalsProg;  // Draw normals

// OBJ shapes
shared_ptr<Shape> plane;
shared_ptr<Shape> rock0;
shared_ptr<Shape> rock1;
shared_ptr<Shape> rock2;
shared_ptr<Shape> rock3;
shared_ptr<Shape> seaweed;
shared_ptr<Shape> wreck;
shared_ptr<Shape> iver_fins;
shared_ptr<Shape> iver_noseAndTail;
shared_ptr<Shape> iver_body;
shared_ptr<Shape> iver_rudder;
shared_ptr<Shape> xlighter;

// Wrapper classes to pass multiple objs to higher order classes
shared_ptr<BubblesShapes> bubblesShapes;

// Textures
shared_ptr<Texture> sandTex;
shared_ptr<Texture> coral0Tex;
shared_ptr<Texture> coral1Tex;
shared_ptr<Texture> coral2Tex;
shared_ptr<Texture> wreckTex;
shared_ptr<Texture> xlighterTex;
shared_ptr<Texture> water_texture1;
shared_ptr<Texture> water_texture2;

// Object transforms handled in this thread
vector<Matrix4f> rockTransforms;
vector<Matrix4f> seaweedTransforms;

// Higher order obj classes that handle some of their own movement
vector<Bubbles> bubbles;

// Camera parameters
double mouse[2];
double alpha, beta, alphaClamp;
Vector3f camPos;
Vector3f camDir;
Vector3f globallightPos;
double speed;
float viewDist;
float g_time = 0;
int curIter = 0;
int pathLength = 0;
int curPathNum = 0;

// Window size parameters
int actualWidth, actualHeight;

// Caustic parameters, constants & textures
Vector3f caust_camPos;
Vector3f caust_camDir;
const int numWaterTextures = 32;
const double elapse = 0.16;
double prevTime;
float caustInterp = 0;
int curWater;
vector<shared_ptr<Texture> > water;

// For image processing
// double thirdsTotal = 0;
// int thirdsIteration = 0;

// Iterative image processing
Node *curNode, *newNode;
const double weightThreshNorm = 0.25;
const double weightThreshThirds = 0.04;
const double weightThreshCombo = (1 - weightThreshThirds) + weightThreshNorm;
int splineNum = 450 / totalPathLen;
// Root node creation
bool generatingRootNode = true;
int rootIter = 0;
Node *bestRootNode = NULL;
double bestRootWeight = 0;

// Vector of camera positions and directions
// initialized at the beginning if playing a path from a file
vector<Vector3f> camPosVec;
vector<Vector3f> camDirVec;
string pathFileName;

// Framebuffer and texture to render to
GLuint framebuffer, renderTexture, depthRenderBuffer;
GLubyte myFrame[1024*768*4];

// Geometry for texture render
GLuint quad_VertexArrayID;
GLuint quad_vertexbuffer;

// VAO and VBO for circular path
GLuint pathsVertArrayObj;
GLuint pathsVertBufObj;


void setCamPos6dof(Vector3f pos, Vector3f dir) {
    camPos = pos;
    camDir = dir;
}

void setCamPos3dof(Vector3f pos) {
    camPos = pos;
}

// Initializes camera arrays from path file
void initCamPath() {
   string fileLocation;
   if (pathFileName.length() > 0) {
      fileLocation = RESOURCE_DIR + pathFileName;
   }
   else {
      fileLocation = RESOURCE_DIR + "path.txt";
   }
   ifstream pathFile(fileLocation);
   if (!pathFile.is_open()) {
      std::cerr << "Cannot open path input file" << std::endl;
      exit(1);
   }
   // Read path length
   pathFile >> pathLength;

   // Initalize temporary position & direction vectors from nodes in file
   vector<Vector3f> tempPosVec;
   vector<Vector3f> tempDirVec;

   float pX, pY, pZ, dX, dY, dZ;
   for (int i = 0; i < pathLength; i++) {
      pathFile >> pX >> pY >> pZ >> dX >> dY >> dZ;

      tempPosVec.push_back(Vector3f(pX, pY, pZ));
      tempDirVec.push_back(Vector3f(dX, dY, dZ));
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
      
         camPosVec.push_back(Gpos*B*uVec);
         camDirVec.push_back(Gdir*B*uVec);       
      }
   }

   // Set camera to first position in path
   setCamPos6dof(camPosVec[0], camDirVec[0]);
}

// Fill the world with underwater objects
static void generate() {
   // Clear out the existing world
   bubbles.clear();
   rockTransforms.clear();
   seaweedTransforms.clear();

   // Initialize random bubbles
   int numBubbles = 600;
   bubblesShapes = make_shared<BubblesShapes>(RESOURCE_DIR);
   for (int i = 0; i < numBubbles; i++) {
      Bubbles bubble = Bubbles(bubblesShapes);
      bubbles.push_back(bubble);
   }

   // Initialize random rock transforms
   int numRock0 = 500;
   Vector2f rSC(3.0f, 10.0f);
   Vector2f rR(30.0f, 300.0f);
   for (int i = 0; i < numRock0; i++) {
      auto E = make_shared<MatrixStack>();
      E->loadIdentity();
      float sc = randRangef(rSC(0), rSC(1));
      float r = randRangef(rR(0), rR(1));
      float rot = randRangef(0.0f, 2.0f * (float)M_PI);
      float theta = randRangef(0.0f, 2.0f * (float)M_PI);

      E->translate(Vector3f(r * cos(theta), 0.0f, r * sin(theta)));
      E->rotate(rot, Vector3f(0.0f, 1.0f, 0.0f));
      E->scale(sc);

      rockTransforms.push_back(E->topMatrix());
   }

   // Initialize random seaweed transforms
   int numSeaweed = 100;
   Vector2f swSC(5.0f, 10.0f);
   Vector2f swR(15.0f, 300.0f);
   Vector2f swDR(0.0f, 0.001f);
   Vector2f swDensity(3.0f, 10.0f);
   for (int i = 0; i < numSeaweed; i++) {
      float r = randRangef(swR(0), swR(1));
      float theta = randRangef(0.0f, 2.0f * (float)M_PI);

      int num = (int)randRangef(swDensity(0), swDensity(1));
      for (int j = 0; j < num; j++) {
         float sc = randRangef(swSC(0), swSC(1));
         float rot = randRangef(0.0f, 2.0f * (float)M_PI);
         float dR = randRangef(swDR(0), swDR(1));
         float dTheta = randRangef(0.0f, 2.0f * (float)M_PI);
         
         auto E = make_shared<MatrixStack>();
         E->loadIdentity();
         E->translate(Vector3f(r * cos(theta), 0.0f, r * sin(theta)));
         E->translate((Vector3f(dR * cos(dTheta), 0.0f, dR * sin(dTheta))));
         E->rotate(rot, Vector3f(0.0f, 1.0f, 0.0f));
         E->scale(sc);
         seaweedTransforms.push_back(E->topMatrix());
     }
   }
}

static void error_callback(int error, const char *description) {
   cerr << description << endl;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
   if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
      glfwSetWindowShouldClose(window, GL_TRUE);
   }

   // Move the camera around
   else if (key == GLFW_KEY_W) {
      camPos += speed * camDir;
   }
   else if (key == GLFW_KEY_A) {
      Vector3f camCross = camDir.cross(Vector3f(0.0, 1.0, 0.0)).normalized();
      camPos -= camCross;
   }
   else if (key == GLFW_KEY_S) {
      camPos -= speed * camDir;
   }
   else if (key == GLFW_KEY_D) {
      Vector3f camCross = camDir.cross(Vector3f(0.0, 1.0, 0.0)).normalized();
      camPos += camCross;
   }
   else if (key == GLFW_KEY_SPACE) {
      camPos += speed * Vector3f(0.0, 1.0, 0.0);
   }
   // Re-generate the world
   else if (key == GLFW_KEY_G && action == GLFW_PRESS) {
      generate();
   }
   else if (key == GLFW_KEY_J) {
      globallightPos[2] += 0.5;
   }
   else if (key == GLFW_KEY_L) {
      globallightPos[2] -= 0.5;
   }
   else if (key == GLFW_KEY_K) {
      globallightPos[1] -= 0.5;
   }
   else if (key == GLFW_KEY_I) {
      globallightPos[1] += 0.5;
   }
   else if (key == GLFW_KEY_U) {
      globallightPos[0] -= 0.5;
   }
   else if (key == GLFW_KEY_O) {
      globallightPos[0] += 0.5;
   } else if(key == GLFW_KEY_B){
      cout << globallightPos[0] << " " 
         << globallightPos[1] << " " 
         << globallightPos[2] << endl;
   }
}

static void char_callback(GLFWwindow *window, unsigned int key) {
   keyToggles[key] = !keyToggles[key];
}


static void mouse_callback(GLFWwindow *window, int button, int action, int mods) {
   double posX, posY;
   if (action == GLFW_PRESS) {
      glfwGetCursorPos(window, &posX, &posY);
      mouse[0] = posX;
      mouse[1] = posY;
   }
}

static void cursor_pos_callback(GLFWwindow * window, double xpos, double ypos) {
   int mouseAction = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
   if (mouseAction == GLFW_PRESS) {
      double posX, posY;
      glfwGetCursorPos(window, &posX, &posY);
      double dX = posX - mouse[0];
      double dY = posY - mouse[1];

      int width, height;
      glfwGetFramebufferSize(window, &width, &height);

      // Calculate the camera angles
      alpha -= 0.5 * M_PI * dY / (double)width;
      beta += M_PI * dX / (double)height;

      // Clamp alpha between +/- 85 degrees.
      if (alpha > alphaClamp) {
         alpha = alphaClamp;
      }
      else if (alpha < -alphaClamp) {
         alpha = -alphaClamp;
      }

      // Calculate the new camera direction
      camDir(0) = cos(alpha) * cos(beta);
      camDir(1) = sin(alpha);
      camDir(2) = cos(alpha) * cos(0.5 * M_PI - beta);

      // Record the x and y positions
      mouse[0] = posX;
      mouse[1] = posY;
   }
}

static void resize_callback(GLFWwindow *window, int width, int height) { 
   glViewport(0, 0, width, height);
}

// Set materials to objs
void setMaterial(int i, shared_ptr<Program> prog) {
   switch (i) {
      case 0: // bubbles
         glUniform3f(prog->getUniform("matAmb"), 0.9f, 0.9f, 0.9f);
         glUniform3f(prog->getUniform("matDif"), 0.0f, 0.0f, 0.0f);
         glUniform3f(prog->getUniform("matSpec"), 0.2f, 0.2f, 0.9f);
         glUniform1f(prog->getUniform("matShine"), 20.0f);
         break;
      case 1: // seaweed
         glUniform3f(prog->getUniform("matAmb"), 0.0f, 0.2f, 0.1f);
         glUniform3f(prog->getUniform("matDif"), 0.0f, 0.4f, 0.0f);
         glUniform3f(prog->getUniform("matSpec"), 0.4f, 0.5f, 0.4f);
         glUniform1f(prog->getUniform("matShine"), 20.0f);
         break;
      case 2: //iver body and iver rudder
         glUniform3f(prog->getUniform("matAmb"), 0.1f, 0.1f, 0.1f);
         glUniform3f(prog->getUniform("matDif"), 0.64f, 0.64f, 0.64f);
         glUniform3f(prog->getUniform("matSpec"), 0.5f, 0.5f, 0.5f);
         glUniform1f(prog->getUniform("matShine"), 5.0f);
         break;
      case 3: //iver noseAndTail
         glUniform3f(prog->getUniform("matAmb"), 0.021768f, 0.021768f, 0.021768f);
         glUniform3f(prog->getUniform("matDif"), 0.021768f, 0.021768f, 0.021768f);
         glUniform3f(prog->getUniform("matSpec"), 0.055556f, 0.055556f, 0.055556f);
         glUniform1f(prog->getUniform("matShine"), 5.0f);
         break;
      case 4: //iver fins
         glUniform3f(prog->getUniform("matAmb"), 0.64f, 0.253291f, 0.0f);
         glUniform3f(prog->getUniform("matDif"), 0.64f, 0.253291f, 0.0f);
         glUniform3f(prog->getUniform("matSpec"), 0.5f, 0.5f, 0.5f);
         glUniform1f(prog->getUniform("matShine"), 5.0f);
         break;
      }
}

// Set materials to textured objs
void setTextureMaterial(int i, shared_ptr<Program> prog) {
   switch (i) {
      case 0: // sand
         glUniform1f(prog->getUniform("matAmb"), 0.25f);
         glUniform1f(prog->getUniform("matDif"), 1.0f);
         glUniform3f(prog->getUniform("matSpec"), 0.3f, 0.3f, 0.3f);
         glUniform1f(prog->getUniform("matShine"), 5.0f);
         break;
      case 1: // water surface
         glUniform1f(prog->getUniform("matAmb"), 0.25f);
         glUniform1f(prog->getUniform("matDif"), 0.3f);
         glUniform3f(prog->getUniform("matSpec"), 0.2f, 0.2f, 0.2f);
         glUniform1f(prog->getUniform("matShine"), 5.0f);
         break;
      case 2: // coral
         glUniform1f(prog->getUniform("matAmb"), 0.25f);
         glUniform1f(prog->getUniform("matDif"), 0.6f);
         glUniform3f(prog->getUniform("matSpec"), 0.1f, 0.1f, 0.1f);
         glUniform1f(prog->getUniform("matShine"), 5.0f);
         break;
      case 3: // ICEX wreck
         glUniform1f(prog->getUniform("matAmb"), 1.0f);
         glUniform1f(prog->getUniform("matDif"), 1.0f);
         glUniform3f(prog->getUniform("matSpec"), 0.0f, 0.0f, 0.0f);
         glUniform1f(prog->getUniform("matShine"), 1.0f);
         break;
      case 4: // ICEX Xlighter wreck
         glUniform1f(prog->getUniform("matAmb"), 0.25f);
         glUniform1f(prog->getUniform("matDif"), 1.0f);
         glUniform3f(prog->getUniform("matSpec"), 0.3f, 0.3f, 0.3f);
         glUniform1f(prog->getUniform("matShine"), 5.0f);
         break;
   }
}

static void init() {
   GLSL::checkVersion();

   // Initialize camera variables
   viewDist = 300.0f;
   speed = 0.75;
   alpha = 0.0;
   alphaClamp = 85.0 * M_PI / 180.0;
   beta = 0.5 * M_PI;
   camPos = Vector3f(0.0, 1.0, -15.0);
   camDir = Vector3f(0.0, 0.0, 1.0);
   globallightPos = Vector3f(0.0f, 42.5f, -1.0f);
   caust_camPos = Vector3f(0.0, 30.0, 0.0);
   caust_camDir = Vector3f(0.0, -1.0, 0.0);

   // Set background color.
   Vector3f bGColor = Vector3f(.011f, 0.2f, 0.47f);
   glClearColor(bGColor[0], bGColor[1], bGColor[2], 1.0f);

   // Enable z-buffer test.
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glEnable(GL_CULL_FACE);
   glCullFace(GL_BACK);
   glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

   // Initalize the camera path if we are playing a path from a file
   if (playPaths) {
      initCamPath();
   }
   else { // Create the root node if we are generating a path
      curNode = generateRootPRMNode();
      bestRootWeight = genThirds ? 1 : 0;
   }

   // Init additions for ICEX
   // Generate framebuffer, texture, and depth renderbuffer
   glfwGetFramebufferSize(window, &actualWidth, &actualHeight);

   glGenFramebuffers(1, &framebuffer);
   glGenTextures(1, &renderTexture);
   glGenRenderbuffers(1, &depthRenderBuffer);
   glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
   glBindTexture(GL_TEXTURE_2D, depthRenderBuffer);
   glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);

   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, actualWidth, actualHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, 0);

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

   glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderTexture, 0);

   glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, actualWidth, actualHeight);
   glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);

   GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
   glDrawBuffers(1, DrawBuffers);

   if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      cout << "framebuffer error" << endl;
   }

   // Set up a simple quad for rendering FBO
   glGenVertexArrays(1, &quad_VertexArrayID);
   glBindVertexArray(quad_VertexArrayID);

   static const GLfloat g_quad_vertex_buffer_data[] = {
      -1.0f, -1.0f, 0.0f,
      1.0f, -1.0f, 0.0f,
      -1.0f,  1.0f, 0.0f,
      -1.0f,  1.0f, 0.0f,
      1.0f, -1.0f, 0.0f,
      1.0f,  1.0f, 0.0f,
   };

   glGenBuffers(1, &quad_vertexbuffer);
   glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
   glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), 
      g_quad_vertex_buffer_data, GL_STATIC_DRAW);

   // Initalizations necessary to draw camera path
   glGenVertexArrays(1, &pathsVertArrayObj);
   glGenBuffers(1, &pathsVertBufObj);
   glBindVertexArray(pathsVertArrayObj);
   glBindBuffer(GL_ARRAY_BUFFER, pathsVertBufObj);
   glBufferData(GL_ARRAY_BUFFER, camPosVec.size() * sizeof(GLfloat) * 3, 
      &camPosVec[0], GL_STATIC_DRAW);

   GLenum error = glGetError();
   if (error != GL_NO_ERROR) {
      cout << "error vertices: " << error << endl;
   }

   tex_prog = make_shared<Program>();
   tex_prog->setVerbose(true);
   tex_prog->setShaderNames(RESOURCE_DIR + "pass_vert.glsl", RESOURCE_DIR + "tex_frag.glsl");
   tex_prog->init();
   tex_prog->addUniform("texBuf");
   tex_prog->addAttribute("vertPos");

   // Initialize plane for textures.
   plane = make_shared<Shape>();
   plane->loadMesh(RESOURCE_DIR + "plane.obj");
   plane->resize();
   plane->init();

   // Initialize rock 0.
   rock0 = make_shared<Shape>();
   rock0->loadMesh(RESOURCE_DIR + "Boulder1.obj");
   rock0->resize();
   rock0->init();

   // Initialize rock 1.
   rock1 = make_shared<Shape>();
   rock1->loadMesh(RESOURCE_DIR + "Boulder2.obj");
   rock1->resize();
   rock1->init();

   // Initialize rock 2.
   rock2 = make_shared<Shape>();
   rock2->loadMesh(RESOURCE_DIR + "Boulder3.obj");
   rock2->resize();
   rock2->init();

   // Initialize rock 3.
   rock3 = make_shared<Shape>();
   rock3->loadMesh(RESOURCE_DIR + "rockArc.obj");
   rock3->resize();
   rock3->moveToGround();
   rock3->init();

   // Initialize seaweed.
   seaweed = make_shared<Shape>();
   seaweed->loadMesh(RESOURCE_DIR + "seaweed.obj");
   seaweed->resize();
   seaweed->moveToGround();
   seaweed->init();

   // Initialize ICEX wreck
   wreck = make_shared<Shape>();
   wreck->loadMesh(RESOURCE_DIR + "chimney.obj");
   wreck->resize();
   wreck->init();

   // Initialize ICEX xlighter
   xlighter = make_shared<Shape>();
   xlighter->loadMesh(RESOURCE_DIR + "ManoelPRMsCombined.obj");
   xlighter->resize();
   xlighter->init();

   //initialize ICEX iver parts
   //intialize iver_fins
   iver_fins = make_shared<Shape>();
   iver_fins->loadMesh(RESOURCE_DIR + "iver_fins.obj");
   //iver_fins->resize();
   iver_fins->init();

   //initialize iver_body
   iver_body = make_shared<Shape>();
   iver_body->loadMesh(RESOURCE_DIR + "iver_body.obj");
   iver_body->init();

   //initialize iver_rudder
   iver_rudder = make_shared<Shape>();
   iver_rudder->loadMesh(RESOURCE_DIR + "iver_rudder.obj");
   iver_rudder->init();

   //intialize iver_noseAndTail
   iver_noseAndTail = make_shared<Shape>();
   iver_noseAndTail->loadMesh(RESOURCE_DIR + "iver_noseAndTail.obj");
   //iver_noseAndTail->resize();
   iver_noseAndTail->init();

   // Initialize sand texture.
   sandTex = make_shared<Texture>();
   sandTex->setFilename(RESOURCE_DIR + "sandDark.png");
   sandTex->init();

   // Initialize coral texture 0.
   coral0Tex = make_shared<Texture>();
   coral0Tex->setFilename(RESOURCE_DIR + "coral0.png");
   coral0Tex->init();

   // Initialize coral texture 1.
   coral1Tex = make_shared<Texture>();
   coral1Tex->setFilename(RESOURCE_DIR + "coral1.png");
   coral1Tex->init();

   // Initialize coral texture 2.
   coral2Tex = make_shared<Texture>();
   coral2Tex->setFilename(RESOURCE_DIR + "coral2.png");
   coral2Tex->init();

   // Initalize the ICEX wreck texture
   wreckTex = make_shared<Texture>();
   wreckTex->setFilename(RESOURCE_DIR + "chimney.jpg");
   wreckTex->init();

   // Initalize the ICEX Xlighter wreck texture
   xlighterTex = make_shared<Texture>();
   xlighterTex->setFilename(RESOURCE_DIR + "ManoelPRMsCombined.jpg");
   xlighterTex->init();

   // Generate the world
   generate();

   //initialize the blinn-phong program
   phongProg = make_shared<Program>();
   phongProg->setVerbose(true);
   phongProg->setShaderNames(RESOURCE_DIR + "phong_vert.glsl", RESOURCE_DIR + "phong_frag.glsl");
   phongProg->init();
   phongProg->addUniform("P");
   phongProg->addUniform("V");
   phongProg->addUniform("M");
   phongProg->addUniform("camPos");
   phongProg->addUniform("lightPos1");
   phongProg->addUniform("lightPos2");
   phongProg->addUniform("lightCol1");
   phongProg->addUniform("lightCol2");
   phongProg->addUniform("matAmb");
   phongProg->addUniform("matDif");
   phongProg->addUniform("matSpec");
   phongProg->addUniform("matShine");
   phongProg->addAttribute("vertPos");
   phongProg->addAttribute("vertNor");
   phongProg->addUniform("caust_V");
   phongProg->addUniform("water1");
   phongProg->addUniform("water2");
   phongProg->addUniform("interp");

   // Initialize the fading blinn-phong program
   fadePhongProg = make_shared<Program>();
   fadePhongProg->setVerbose(true);
   fadePhongProg->setShaderNames(RESOURCE_DIR + "fading_phong_vert.glsl", RESOURCE_DIR + "fading_phong_frag.glsl");
   fadePhongProg->init();
   fadePhongProg->addUniform("P");
   fadePhongProg->addUniform("V");
   fadePhongProg->addUniform("M");
   fadePhongProg->addUniform("camPos");
   fadePhongProg->addUniform("lightPos1");
   fadePhongProg->addUniform("lightPos2");
   fadePhongProg->addUniform("lightCol1");
   fadePhongProg->addUniform("lightCol2");
   fadePhongProg->addUniform("viewDist");
   fadePhongProg->addUniform("matAmb");
   fadePhongProg->addUniform("matDif");
   fadePhongProg->addUniform("matSpec");
   fadePhongProg->addUniform("matShine");
   fadePhongProg->addUniform("baseAlpha");
   fadePhongProg->addAttribute("vertPos");
   fadePhongProg->addAttribute("vertNor");
   fadePhongProg->addUniform("caust_V");
   fadePhongProg->addUniform("water1");
   fadePhongProg->addUniform("water2");
   fadePhongProg->addUniform("interp");

   // Initialize the fading, textured, blinn-phong program
   fadeTexPhongProg = make_shared<Program>();
   fadeTexPhongProg->setVerbose(true);
   fadeTexPhongProg->setShaderNames(RESOURCE_DIR + "fading_tex_phong_vert.glsl", RESOURCE_DIR + "fading_tex_phong_frag.glsl");
   fadeTexPhongProg->init();
   fadeTexPhongProg->addUniform("P");
   fadeTexPhongProg->addUniform("V");
   fadeTexPhongProg->addUniform("M");
   fadeTexPhongProg->addUniform("camPos");
   fadeTexPhongProg->addUniform("lightPos1");
   fadeTexPhongProg->addUniform("lightPos2");
   fadeTexPhongProg->addUniform("lightCol1");
   fadeTexPhongProg->addUniform("lightCol2");
   fadeTexPhongProg->addUniform("matAmb");
   fadeTexPhongProg->addUniform("matDif");
   fadeTexPhongProg->addUniform("matSpec");
   fadeTexPhongProg->addUniform("matShine");
   fadeTexPhongProg->addUniform("texture0");
   fadeTexPhongProg->addAttribute("vertPos");
   fadeTexPhongProg->addAttribute("vertNor");
   fadeTexPhongProg->addAttribute("vertTex");
   fadeTexPhongProg->addUniform("caust_V");
   fadeTexPhongProg->addUniform("water1");
   fadeTexPhongProg->addUniform("water2");
   fadeTexPhongProg->addUniform("caust");
   fadeTexPhongProg->addUniform("isAgisoftModel");
   fadeTexPhongProg->addUniform("interp");

   // Initialize the fading, waving, blinn-phong program
   fadeWavePhongProg = make_shared<Program>();
   fadeWavePhongProg->setVerbose(true);
   fadeWavePhongProg->setShaderNames(RESOURCE_DIR + "fading_waving_phong_vert.glsl", RESOURCE_DIR + "fading_waving_phong_frag.glsl");
   fadeWavePhongProg->init();
   fadeWavePhongProg->addUniform("P");
   fadeWavePhongProg->addUniform("V");
   fadeWavePhongProg->addUniform("M");
   fadeWavePhongProg->addUniform("camPos");
   fadeWavePhongProg->addUniform("lightPos1");
   fadeWavePhongProg->addUniform("lightPos2");
   fadeWavePhongProg->addUniform("lightCol1");
   fadeWavePhongProg->addUniform("lightCol2");
   fadeWavePhongProg->addUniform("viewDist");
   fadeWavePhongProg->addUniform("matAmb");
   fadeWavePhongProg->addUniform("matDif");
   fadeWavePhongProg->addUniform("matSpec");
   fadeWavePhongProg->addUniform("matShine");
   fadeWavePhongProg->addUniform("t");
   fadeWavePhongProg->addUniform("wave");
   fadeWavePhongProg->addAttribute("vertPos");
   fadeWavePhongProg->addAttribute("vertNor");
   fadeWavePhongProg->addUniform("caust_V");
   fadeWavePhongProg->addUniform("water1");
   fadeWavePhongProg->addUniform("water2");
   fadeWavePhongProg->addUniform("interp");

   // Initialize the solid color program
   solidColorProg = make_shared<Program>();
   solidColorProg->setVerbose(true);
   solidColorProg->setShaderNames(RESOURCE_DIR + "solid_color_vert.glsl", RESOURCE_DIR + "solid_color_frag.glsl");
   solidColorProg->init();
   solidColorProg->addUniform("P");
   solidColorProg->addUniform("V");
   solidColorProg->addUniform("M");
   solidColorProg->addAttribute("vertPos");

   // Initialize the normals program
   normalsProg = make_shared<Program>();
   normalsProg->setVerbose(true);
   normalsProg->setShaderNames(RESOURCE_DIR + "normals_vert.glsl", RESOURCE_DIR + "normals_frag.glsl");
   normalsProg->init();
   normalsProg->addUniform("P");
   normalsProg->addUniform("V");
   normalsProg->addUniform("M");
   normalsProg->addAttribute("vertPos");
   normalsProg->addAttribute("vertNor");
   normalsProg->addAttribute("vertTex");

   // Initalize caustics textures
   for (int i = 1; i <= numWaterTextures; i++) {
      shared_ptr<Texture> water_texture = make_shared<Texture>();
      string num = std::to_string(i);
      water_texture->setFilename(RESOURCE_DIR + "save." + num + ".jpeg");
      water_texture->init();
      water_texture->setWrapModes(GL_REPEAT, GL_REPEAT);
      water.push_back(water_texture);
   }
   prevTime = glfwGetTime();
   curWater = 0;
   water_texture1 = water[curWater];
   water_texture2 = water[(curWater + 1) % numWaterTextures];
}

void drawRocks(shared_ptr<MatrixStack> &M) {
   setTextureMaterial(2, fadeTexPhongProg);

   for (int i = 0; i < (int)rockTransforms.size() / 5; i++) {
      coral0Tex->bind(fadeTexPhongProg->getUniform("texture0"), 0);
      // Switch textures at the 1 / 3 and 2 / 3 marks.
      if (i % 3 == 1) {
         coral0Tex->unbind(0);
         coral1Tex->bind(fadeTexPhongProg->getUniform("texture0"), 0);
      }
      else if (i % 3 == 2) {
         coral1Tex->unbind(0);
         coral2Tex->bind(fadeTexPhongProg->getUniform("texture0"), 0);
      }

      M->pushMatrix();
      M->scale(0.2);
      M->multMatrix(rockTransforms[i]);
      glUniformMatrix4fv(fadeTexPhongProg->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());
      switch (i % 4) {
         case 0:
            rock0->draw(fadeTexPhongProg);
            break;
         case 1:
            rock1->draw(fadeTexPhongProg);
            break;
         case 2:
            rock2->draw(fadeTexPhongProg);
            break;
         case 3:
            rock3->draw(fadeTexPhongProg);
            break;
      }
      M->popMatrix();
   }
   coral2Tex->unbind(0);
}

void drawSand(shared_ptr<MatrixStack> &M, Vector2i &gridLL, Vector2i &gridUR, float scale) {
   sandTex->bind(fadeTexPhongProg->getUniform("texture0"), 0);
   setTextureMaterial(0, fadeTexPhongProg);
   for (int i = gridLL(0); i < gridUR(0); i++) {
      for (int j = gridLL(1); j < gridUR(1); j++) {
         M->pushMatrix();
         M->translate(Vector3f(2.0f * scale * (float)i, 0.0f, 2.0f * scale * (float)j));
         M->scale(scale);
         glUniformMatrix4fv(fadeTexPhongProg->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());
         plane->draw(fadeTexPhongProg);
         M->popMatrix();
      }
   }
   sandTex->unbind(0);
}

void drawScenery(shared_ptr<MatrixStack> &M) {
   drawRocks(M);

   Vector2i gridLL(-20, -20 );
   Vector2i gridUR(20, 20);
   float scale = 10.0f;
   drawSand(M, gridLL, gridUR, scale);
}

void drawChimChiminy(shared_ptr<MatrixStack> &M) {
   wreckTex->bind(fadeTexPhongProg->getUniform("texture0"), 0);
   setTextureMaterial(3, fadeTexPhongProg);
   M->pushMatrix();
   M->translate(Vector3f(0, 2, 0));
   M->rotate(M_PI, Vector3f(1, 0, 0));
   M->scale(8);
   glUniformMatrix4fv(fadeTexPhongProg->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());
   wreck->draw(fadeTexPhongProg);
   M->popMatrix();
   wreckTex->unbind(0);
}

void drawBeaufighter(shared_ptr<MatrixStack> &M) {
   wreckTex->bind(fadeTexPhongProg->getUniform("texture0"), 0);
   setTextureMaterial(3, fadeTexPhongProg);
   M->pushMatrix();
   M->translate(Vector3f(0, 2, 0));
   M->rotate(M_PI / 2, Vector3f(1, 0, 0));
   M->scale(8);
   glUniformMatrix4fv(fadeTexPhongProg->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());
   wreck->draw(fadeTexPhongProg);
   M->popMatrix();
   wreckTex->unbind(0);
}

void drawXlighter(shared_ptr<MatrixStack> &M) {
   xlighterTex->bind(fadeTexPhongProg->getUniform("texture0"), 0);
   glUniform1i(fadeTexPhongProg->getUniform("isAgisoftModel"), 1);
   setTextureMaterial(4, fadeTexPhongProg);
   M->pushMatrix();
   // M->translate(Vector3f(0, 2, -1));
   M->translate(Vector3f(6, 2, 0));
   M->rotate(-30 * M_PI / 180.0f, Vector3f(0, 1, 0));
   M->rotate(90 * M_PI / 180.0f, Vector3f(1, 0, 0));
   M->rotate(-45 * M_PI / 180.0f, Vector3f(0, 1, 0));
   M->rotate(10 * M_PI / 180.0f, Vector3f(0, 0, 1));
   M->rotate(20 * M_PI / 180.0f, Vector3f(0, 1, 0));
   M->scale(8);
   glUniformMatrix4fv(fadeTexPhongProg->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());
   xlighter->draw(fadeTexPhongProg);
   M->popMatrix();
   xlighterTex->unbind(0);
}

void drawIver(shared_ptr<MatrixStack> &P, shared_ptr<MatrixStack> &V, shared_ptr<MatrixStack> &M,
              shared_ptr<MatrixStack> &caust_V, float lightPos[], float lightCol[], double t) {
   phongProg->bind();

   glUniformMatrix4fv(phongProg->getUniform("P"), 1, GL_FALSE, P->topMatrix().data());
   glUniformMatrix4fv(phongProg->getUniform("V"), 1, GL_FALSE, V->topMatrix().data());
   glUniform3f(phongProg->getUniform("camPos"), (float) camPos[0], (float) camPos[1], (float) camPos[2]);
   glUniform3f(phongProg->getUniform("lightPos1"), lightPos[0], lightPos[1], lightPos[2]);
   glUniform3f(phongProg->getUniform("lightPos2"), lightPos[3], lightPos[4], lightPos[5]);
   glUniform3f(phongProg->getUniform("lightCol1"), lightCol[0], lightCol[1], lightCol[2]);
   glUniform3f(phongProg->getUniform("lightCol2"), lightCol[3], lightCol[4], lightCol[5]);
   water_texture1->bind(phongProg->getUniform("water1"), curWater + 6);
   water_texture2->bind(phongProg->getUniform("water2"), curWater + 7);
   glUniform1f(phongProg->getUniform("interp"), caustInterp);
   glUniformMatrix4fv(phongProg->getUniform("caust_V"), 1, GL_FALSE, caust_V->topMatrix().data());

   setMaterial(2, phongProg);

   M->pushMatrix();
      // M->translate(Vector3f(5, 5, 5));
      M->rotate(g_time, Vector3f(0, 1, 0));
      M->translate(Vector3f(5.0f, 8.0f + (2 * sin(g_time)), 5.0f));
      M->rotate(-65 * M_PI/180.0f, Vector3f(0, 1, 0));
      M->rotate(25 * M_PI/180.0f * cos(g_time), Vector3f(1, 0, 0));
      M->scale(.35);
      glUniformMatrix4fv(phongProg->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());
      iver_body->draw(phongProg);
      M->pushMatrix();
         M->translate(Vector3f(0, 0, 4));
         M->rotate(-10 * M_PI/180.0f + (5 * M_PI/180.0f * sin(2 * g_time)), Vector3f(0, 1, 0));
         M->translate(Vector3f(0, 0, -4));
         glUniformMatrix4fv(phongProg->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());
         iver_rudder->draw(phongProg);
      M->popMatrix();
      setMaterial(3, phongProg);
      glUniformMatrix4fv(phongProg->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());
      iver_noseAndTail->draw(phongProg);
      M->pushMatrix();
         M->translate(Vector3f(0, 0, 4));
         M->rotate(-20 * M_PI/180.0f * cos(g_time), Vector3f(1, 0, 0));
         M->translate(Vector3f(0, 0, -4));
         glUniformMatrix4fv(phongProg->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());
         setMaterial(4, phongProg);
         iver_fins->draw(phongProg);
      M->popMatrix();
   M->popMatrix();

   water_texture1->unbind(curWater + 6);
   water_texture2->unbind(curWater + 7);
   phongProg->unbind();
}

/* Draw objects that:
 1. Are textured objs
 2. Need blinn-phong lighting
 3. Fade in the distance
 */
void drawTexturedObjects(shared_ptr<MatrixStack> &P, shared_ptr<MatrixStack> &V, shared_ptr<MatrixStack> &M,
                         shared_ptr<MatrixStack> &caust_V, float lightPos[], float lightCol[]) {
   fadeTexPhongProg->bind();
   glUniformMatrix4fv(fadeTexPhongProg->getUniform("P"), 1, GL_FALSE, P->topMatrix().data());
   glUniformMatrix4fv(fadeTexPhongProg->getUniform("V"), 1, GL_FALSE, V->topMatrix().data());
   glUniform3f(fadeTexPhongProg->getUniform("camPos"), (float) camPos[0], (float) camPos[1], (float) camPos[2]);

   glUniform3f(fadeTexPhongProg->getUniform("lightPos1"), lightPos[0], lightPos[1], lightPos[2]);
   glUniform3f(fadeTexPhongProg->getUniform("lightPos2"), lightPos[3], lightPos[4], lightPos[5]);
   glUniform3f(fadeTexPhongProg->getUniform("lightCol1"), lightCol[0], lightCol[1], lightCol[2]);
   glUniform3f(fadeTexPhongProg->getUniform("lightCol2"), lightCol[3], lightCol[4], lightCol[5]);

   water_texture1->bind(fadeTexPhongProg->getUniform("water1"), curWater + 6);
   water_texture2->bind(fadeTexPhongProg->getUniform("water2"), curWater + 7);
   glUniform1f(fadeTexPhongProg->getUniform("interp"), caustInterp);
   glUniformMatrix4fv(fadeTexPhongProg->getUniform("caust_V"), 1, GL_FALSE, caust_V->topMatrix().data());
   if (playPaths) {
      glUniform1f(fadeTexPhongProg->getUniform("caust"), 1.0f);
   }
   else {
      glUniform1f(fadeTexPhongProg->getUniform("caust"), 0.0f);
   }
   glUniform1i(fadeTexPhongProg->getUniform("isAgisoftModel"), 0);

   drawScenery(M);
   // drawChimChiminy(M);
   // drawBeaufighter(M);
   drawXlighter(M);

   water_texture1->unbind(curWater + 6);
   water_texture2->unbind(curWater + 7);
   fadeTexPhongProg->unbind();
}

/* Draw objects that:
 1. Are waving objs
 2. Need blinn-phong lighting
 3. Fade in the distance
 */
void drawSeaweed(shared_ptr<MatrixStack> &P, shared_ptr<MatrixStack> &V, shared_ptr<MatrixStack> &M,
                 shared_ptr<MatrixStack> &caust_V, float lightPos[], float lightCol[], double t) {
   fadeWavePhongProg->bind();
   glUniformMatrix4fv(fadeWavePhongProg->getUniform("P"), 1, GL_FALSE, P->topMatrix().data());
   glUniformMatrix4fv(fadeWavePhongProg->getUniform("V"), 1, GL_FALSE, V->topMatrix().data());
   glUniform3f(fadeWavePhongProg->getUniform("camPos"), (float) camPos[0], (float) camPos[1], (float) camPos[2]);
   glUniform3f(fadeWavePhongProg->getUniform("lightPos1"), lightPos[0], lightPos[1], lightPos[2]);
   glUniform3f(fadeWavePhongProg->getUniform("lightPos2"), lightPos[3], lightPos[4], lightPos[5]);
   glUniform3f(fadeWavePhongProg->getUniform("lightCol1"), lightCol[0], lightCol[1], lightCol[2]);
   glUniform3f(fadeWavePhongProg->getUniform("lightCol2"), lightCol[3], lightCol[4], lightCol[5]);
   glUniform3f(fadeWavePhongProg->getUniform("wave"), 1.0f, 0.0f, 0.0f);
   glUniform1f(fadeWavePhongProg->getUniform("viewDist"), viewDist);
   glUniform1f(fadeWavePhongProg->getUniform("t"), (float)t);
   water_texture1->bind(fadeWavePhongProg->getUniform("water1"), curWater + 6);
   water_texture2->bind(fadeWavePhongProg->getUniform("water2"), curWater + 7);
   glUniform1f(fadeWavePhongProg->getUniform("interp"), caustInterp);
   glUniformMatrix4fv(fadeWavePhongProg->getUniform("caust_V"), 1, GL_FALSE, caust_V->topMatrix().data());

   // Draw seaweeds
   setMaterial(1, fadeWavePhongProg);
   for (int i = 0; i < (int)seaweedTransforms.size()/3; i++) {
      M->pushMatrix();
      M->scale(0.3);
      M->multMatrix(seaweedTransforms[i]);
      glUniformMatrix4fv(fadeWavePhongProg->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());
      seaweed->draw(fadeWavePhongProg);
      M->popMatrix();
   }

   water_texture1->unbind(curWater + 6);
   water_texture2->unbind(curWater + 7);
   fadeWavePhongProg->unbind();
}

/* Draw objects that:
 1. Are standard objs
 2. Need blinn-phong lighting
 3. Fade in the distance
 */
void drawBubbles(shared_ptr<MatrixStack> &P, shared_ptr<MatrixStack> &V, shared_ptr<MatrixStack> &M,
                 shared_ptr<MatrixStack> &caust_V, float lightPos[], float lightCol[], double t) {
   fadePhongProg->bind();
   glUniformMatrix4fv(fadePhongProg->getUniform("P"), 1, GL_FALSE, P->topMatrix().data());
   glUniformMatrix4fv(fadePhongProg->getUniform("V"), 1, GL_FALSE, V->topMatrix().data());
   glUniformMatrix4fv(fadePhongProg->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());
   glUniform3f(fadePhongProg->getUniform("camPos"), (float) camPos[0], (float) camPos[1], (float) camPos[2]);
   glUniform3f(fadePhongProg->getUniform("lightPos1"), lightPos[0], lightPos[1], lightPos[2]);
   glUniform3f(fadePhongProg->getUniform("lightPos2"), lightPos[3], lightPos[4], lightPos[5]);
   glUniform3f(fadePhongProg->getUniform("lightCol1"), lightCol[0], lightCol[1], lightCol[2]);
   glUniform3f(fadePhongProg->getUniform("lightCol2"), lightCol[3], lightCol[4], lightCol[5]);glUniform1f(fadePhongProg->getUniform("viewDist"), viewDist);
   glUniform1f(fadePhongProg->getUniform("baseAlpha"), 1.0f);
   glUniformMatrix4fv(fadePhongProg->getUniform("caust_V"), 1, GL_FALSE, caust_V->topMatrix().data());
   water_texture1->bind(fadePhongProg->getUniform("water1"), curWater + 6);
   water_texture2->bind(fadePhongProg->getUniform("water2"), curWater + 7);
   glUniform1f(fadePhongProg->getUniform("interp"), caustInterp);

   // Draw Bubbles
   glUniform1f(fadePhongProg->getUniform("baseAlpha"), 0.5f);
   setMaterial(0, fadePhongProg);
   for (unsigned int i = 0; i < bubbles.size(); i++) {
      bubbles[i].draw(fadePhongProg, t);
   }

   water_texture1->unbind(curWater + 6);
   water_texture2->unbind(curWater + 7);
   fadePhongProg->unbind();
}

void drawCircularPath(shared_ptr<MatrixStack> &P, shared_ptr<MatrixStack> &V, shared_ptr<MatrixStack> &M) {
   solidColorProg->bind();
   glUniformMatrix4fv(solidColorProg->getUniform("P"), 1, GL_FALSE, P->topMatrix().data());
   glUniformMatrix4fv(solidColorProg->getUniform("V"), 1, GL_FALSE, V->topMatrix().data());
   glUniformMatrix4fv(solidColorProg->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());

   glBindVertexArray(pathsVertArrayObj);
   glEnableVertexAttribArray(0);
   glBindBuffer(GL_ARRAY_BUFFER, pathsVertBufObj);
   // glBufferData(GL_ARRAY_BUFFER, camPosVec.size() * sizeof(GLfloat) * 3, &camPosVec[0], GL_STATIC_DRAW);

   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *)0);

   glDrawArrays(GL_LINE_STRIP, 0, camPosVec.size());

   glDisableVertexAttribArray(0);
   solidColorProg->unbind();
}

void drawXlighterNormals(shared_ptr<MatrixStack> &P, shared_ptr<MatrixStack> &V, shared_ptr<MatrixStack> &M) {
   normalsProg->bind();
   glUniformMatrix4fv(normalsProg->getUniform("P"), 1, GL_FALSE, P->topMatrix().data());
   glUniformMatrix4fv(normalsProg->getUniform("V"), 1, GL_FALSE, V->topMatrix().data());

   M->pushMatrix();
   M->translate(Vector3f(6, 2, 0));
   M->rotate(-30 * M_PI / 180.0f, Vector3f(0, 1, 0));
   M->rotate(90 * M_PI / 180.0f, Vector3f(1, 0, 0));
   M->rotate(-45 * M_PI / 180.0f, Vector3f(0, 1, 0));
   M->rotate(10 * M_PI / 180.0f, Vector3f(0, 0, 1));
   M->rotate(20 * M_PI / 180.0f, Vector3f(0, 1, 0));
   M->scale(8);
   glUniformMatrix4fv(normalsProg->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());
   xlighter->draw(normalsProg);
   M->popMatrix();

   normalsProg->unbind();
}

/*void writeToTexture(bool iterate) {
   //regardless unbind the FBO
   glBindFramebuffer(GL_FRAMEBUFFER, 0);

   // Clear framebuffer.
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   //draw textured quad
   tex_prog->bind();
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, renderTexture);
   glUniform1i(tex_prog->getUniform("texBuf"), 0);

   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, myFrame);

   // OpenCV Image Processing
   if (iterate) {
      cv::Mat ocvImg = ocvImgFromGlTex(renderTexture);

      thirdsTotal += std::abs(detectThirds(ocvImg) - 0.6666);
      thirdsIteration++;
      if (thirdsIteration == camPosVec.size()) {
         // cout << "path " << curPathNum << endl;
         float thirdsScore = thirdsTotal / thirdsIteration;
         cout << "thirds: " << thirdsScore << endl;
         // if (std::abs(thirdsScore - 0.6666) < std::abs(bestPathWeight - 0.6666)) {
         //    bestPath.clear();
         //    bestPath = curPath;
         //    bestPathWeight = thirdsScore;
         //    bestPathNum = curPathNum;
         // }
         thirdsIteration = 0;
         thirdsTotal = 0;
         curPathNum++;
         // initCamPath();
         curIter = 0;
      }
   }

   glEnableVertexAttribArray(0);
   glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
   glDrawArrays(GL_TRIANGLES, 0, 6);
   glDisableVertexAttribArray(0);
   tex_prog->unbind();
}*/

// Render objects to scene
static void render() {
   // Interpolate between caustics textures
   caustInterp = (glfwGetTime() - prevTime) / elapse;
   if (glfwGetTime() - prevTime >= elapse) {
      curWater = (curWater + 1) % numWaterTextures;
      prevTime = glfwGetTime();
      caustInterp = 0;
      water_texture1 = water[curWater];
      water_texture2 = water[(curWater + 1) % numWaterTextures];
   }

   double t = glfwGetTime();
   g_time += 0.05;
   // Press p to pause and play path
   if (keyToggles['p']) {
      curIter = ((curIter + 1) % camPosVec.size());
      setCamPos6dof(camPosVec[curIter], camDirVec[curIter]);
   }

   // Get current frame buffer size.
   // Important for retina displays!
   glfwGetFramebufferSize(window, &actualWidth, &actualHeight);
   glViewport(0, 0, actualWidth, actualHeight);

   glBindFramebuffer(GL_FRAMEBUFFER, 0);
   // glBindFramebuffer(GL_FRAMEBUFFER, framebuffer); // This goes with writeToTexture call

   // Clear framebuffer.
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   float aspect = actualWidth/(float)actualHeight;

   float lightPos[] = {200.0f, 200.0f, 200.0f,
                     globallightPos[0], globallightPos[1], globallightPos[2]};
   float lightCol[] = {0.55f, 0.65f, 1.0f,
                     0.5f, 0.5f, 0.5f};

   // Create the matrix stacks
   auto P = make_shared<MatrixStack>();
   auto V = make_shared<MatrixStack>();
   auto M = make_shared<MatrixStack>();

   // Apply initial matrix transforms
   P->pushMatrix();
   P->perspective(45.0f, aspect, 0.01f, viewDist);

   // Create the view matrix
   V->pushMatrix();
   Vector3f lookAtPos = camPos + camDir;
   V->lookAt(camPos.cast<float>(), lookAtPos.cast<float>(), Vector3f(0.0f, 1.0f, 0.0f));

   // Push this frame
   M->pushMatrix();

   // Caustic matrix stacks
   auto caust_V = make_shared<MatrixStack>();

   // Apply scene camera transforms
   caust_V->pushMatrix();
   Vector3f caust_lookAtPos = caust_camPos + caust_camDir;
   caust_V->lookAt(caust_camPos.cast<float>(), caust_lookAtPos.cast<float>(), Vector3f(1.0f, 0.0f, 0.0f));

   drawSeaweed(P, V, M, caust_V, lightPos, lightCol, t);
   drawBubbles(P, V, M, caust_V, lightPos, lightCol, t);
   drawTexturedObjects(P, V, M, caust_V, lightPos, lightCol);
   drawIver(P, V, M, caust_V, lightPos, lightCol, t);
   if (keyToggles['l']) {
      drawCircularPath(P, V, M);
   }

   // Pop matrix stacks.
   M->popMatrix();
   V->popMatrix();
   P->popMatrix();
   caust_V->popMatrix();

   // writeToTexture(keyToggles['p']);
}

// Render objects to scene
static void renderThirds() {
   // Interpolate between caustics textures
   caustInterp = (glfwGetTime() - prevTime) / elapse;
   if (glfwGetTime() - prevTime >= elapse) {
      curWater = (curWater + 1) % numWaterTextures;
      prevTime = glfwGetTime();
      caustInterp = 0;
      water_texture1 = water[curWater];
      water_texture2 = water[(curWater + 1) % numWaterTextures];
   }

   double t = glfwGetTime();
   g_time += 0.05;
   // Set the camera position & direction based on current node
   setCamPos6dof(curNode->getPosition(), curNode->getDirection());

   // Get current frame buffer size.
   // Important for retina displays!
   glfwGetFramebufferSize(window, &actualWidth, &actualHeight);
   glViewport(0, 0, actualWidth, actualHeight);

   // Write to the framebuffer so we can transfer the image to OpenCV
   glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

   // Clear framebuffer.
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   float aspect = actualWidth/(float)actualHeight;

   float lightPos[] = {200.0f, 200.0f, 200.0f,
                     globallightPos[0], globallightPos[1], globallightPos[2]};
   float lightCol[] = {0.75f, 0.75f, 1.0f,
                     0.5f, 0.5f, 0.5f};

   // Create the matrix stacks
   auto P = make_shared<MatrixStack>();
   auto V = make_shared<MatrixStack>();
   auto M = make_shared<MatrixStack>();

   // Apply initial matrix transforms
   P->pushMatrix();
   P->perspective(45.0f, aspect, 0.01f, viewDist);

   // Create the view matrix
   V->pushMatrix();
   Vector3f lookAtPos = camPos + camDir;
   V->lookAt(camPos.cast<float>(), lookAtPos.cast<float>(), Vector3f(0.0f, 1.0f, 0.0f));

   // Push this frame
   M->pushMatrix();

   // Caustic matrix stacks
   auto caust_V = make_shared<MatrixStack>();

   // Apply scene camera transforms
   caust_V->pushMatrix();
   Vector3f caust_lookAtPos = caust_camPos + caust_camDir;
   caust_V->lookAt(caust_camPos.cast<float>(), caust_lookAtPos.cast<float>(), Vector3f(1.0f, 0.0f, 0.0f));

   drawSeaweed(P, V, M, caust_V, lightPos, lightCol, t);
   drawBubbles(P, V, M, caust_V, lightPos, lightCol, t);
   drawTexturedObjects(P, V, M, caust_V, lightPos, lightCol);

   // Pop matrix stacks.
   M->popMatrix();
   V->popMatrix();
   P->popMatrix();
   caust_V->popMatrix();

   //regardless unbind the FBO
   glBindFramebuffer(GL_FRAMEBUFFER, 0);

   // Clear framebuffer.
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   //draw textured quad
   tex_prog->bind();
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, renderTexture);
   glUniform1i(tex_prog->getUniform("texBuf"), 0);

   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, myFrame);

   // OpenCV Image Processing
   cv::Mat ocvImg = ocvImgFromGlTex(renderTexture);

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

   glEnableVertexAttribArray(0);
   glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
   glDrawArrays(GL_TRIANGLES, 0, 6);
   glDisableVertexAttribArray(0);
   tex_prog->unbind();
}

static void renderNormals() {
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

   // Create the matrix stacks
   auto P = make_shared<MatrixStack>();
   auto V = make_shared<MatrixStack>();
   auto M = make_shared<MatrixStack>();

   // Apply initial matrix transforms
   P->pushMatrix();
   P->perspective(45.0f, aspect, 0.01f, viewDist);

   // Create the view matrix
   V->pushMatrix();
   Vector3f lookAtPos = camPos + camDir;
   V->lookAt(camPos.cast<float>(), lookAtPos.cast<float>(), Vector3f(0.0f, 1.0f, 0.0f));

   // Push this frame
   M->pushMatrix();

   drawXlighterNormals(P, V, M);

   // Pop matrix stacks.
   M->popMatrix();
   V->popMatrix();
   P->popMatrix();

   // write to texture
   //regardless unbind the FBO
   glBindFramebuffer(GL_FRAMEBUFFER, 0);

   // Clear framebuffer.
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   //draw textured quad
   tex_prog->bind();
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, renderTexture);
   glUniform1i(tex_prog->getUniform("texBuf"), 0);

   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, myFrame);

   // OpenCV Image Processing
   cv::Mat ocvImg = ocvImgFromGlTex(renderTexture);

   double nodeWeight;
   if (genNorms) {
      nodeWeight = detectNormals(ocvImg);
   } 
   else {
      nodeWeight = (1.0 - curNode->getWeight()) + detectNormals(ocvImg);
   }
   if (generatingRootNode) {
      if (nodeWeight > bestRootWeight) {
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
         if (genNorms && bestRootWeight > weightThreshNorm) {
            highWeightNodes.push_back(bestRootNode->getNdx());
         }
         else if (genCombo && bestRootWeight > weightThreshCombo) {
            highWeightNodes.push_back(bestRootNode->getNdx());
         }
         curNode = generatePRMNode();
      }
   }
   else {
      curNode->setWeight(nodeWeight);
      if (genNorms && nodeWeight > weightThreshNorm) {
         highWeightNodes.push_back(curNode->getNdx());
      }
      else if (genCombo && nodeWeight > weightThreshCombo) {
         highWeightNodes.push_back(curNode->getNdx());
      }
      curNode = generatePRMNode();
   }

   glEnableVertexAttribArray(0);
   glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
   glDrawArrays(GL_TRIANGLES, 0, 6);
   glDisableVertexAttribArray(0);
   tex_prog->unbind();
}

int main(int argc, char **argv) {
   srand(time(NULL));

   if (argc > 1) {
      if (strcmp(argv[1], "thirds") == 0) {
         genThirds = true;
      }
      else if (strcmp(argv[1], "norms") == 0) {
         genNorms = true;
      }
      else if (strcmp(argv[1], "combo") == 0) {
         genCombo = true;
      }
      else if (strcmp(argv[1], "paths") == 0) {
         playPaths = true;
      }
      else {
         cout << "Invalid parameters. "
            "Usage: ./ProjF thirds | norms | combo | paths [pathFilename]" << endl;
      exit(1);
      }
      if (argc == 3) {
         pathFileName = argv[2];
      }
   }
   else {
      cout << "Invalid parameters. "
         "Usage: ./ProjF thirds | norms | combo | paths [pathFilename]" << endl;
      exit(1);
   }

   // Set error callback.
   glfwSetErrorCallback(error_callback);
   // Initialize the library.
   if (!glfwInit()) {
      return -1;
   }

   // Request the highest possible version of OGL - important for mac
   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
   if (genThirds || genNorms || genCombo) {
      glfwWindowHint(GLFW_VISIBLE, GL_FALSE); // Hide glfw window
   }

   // Create a windowed mode window and its OpenGL context.
   window = glfwCreateWindow(512, 384, "PRM Camera Paths Thesis", NULL, NULL);
   if (!window) {
      glfwTerminate();
      return -1;
   }
   // Make the window's context current.
   glfwMakeContextCurrent(window);
   // Initialize GLEW.
   glewExperimental = true;
   if (glewInit() != GLEW_OK) {
      cerr << "Failed to initialize GLEW" << endl;
      return -1;
   }
   // Weird bootstrap of glGetError
   glGetError();

   // Set vsync.
   glfwSwapInterval(1);
   // Set keyboard callback.
   glfwSetKeyCallback(window, key_callback);
   // Set char callback.
   glfwSetCharCallback(window, char_callback);
   //set the mouse call back
   glfwSetMouseButtonCallback(window, mouse_callback);
   // Set cursor position callback.
   glfwSetCursorPosCallback(window, cursor_pos_callback);
   //set the window resize call back
   glfwSetFramebufferSizeCallback(window, resize_callback);

   // Initialize scene. Note geometry and paths initialized in init now
   init();

   // Loop until the user closes the window.
   while (!glfwWindowShouldClose(window)) {
      // Render scene.
      if (genThirds) {
         renderThirds();
      }
      else if (genNorms) {
         renderNormals();
      }
      else if (genCombo) {
         renderThirds();
         renderNormals();
      }
      else if (playPaths) {
         render();
      }

      // Swap front and back buffers.
      glfwSwapBuffers(window);
      // Poll for and process events.
      glfwPollEvents();
   }

   // Quit program.
   glfwDestroyWindow(window);
   glfwTerminate();
   return 0;
}
