#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "glm/ext.hpp"

#include "stb_image.h"

#include <cassert>
#include <iostream>
#include <sstream>
#include <cmath>
#include <memory>
#include <vector>
#include <algorithm>
#include <map>
#include <cstdio>

#include <icex_common.hpp>
#include <LevelParser.hpp>
#include <DebugUI.hpp>
#include <Graphics/GLHelper.hpp>

GLFWwindow *init_window();
bool load_icon(GLFWimage *image, const char *path);

GLFWwindow *window;
World *world;

void init() {
    world = new World();

    LevelParser::ParseWorld(world, "../src/world.json");

    world->init();
}

void update(float dt) {
    if (!DebugUI::enabled)
        Mouse::update();    
    world->update(dt);
}

void render(float dt) {
    world->render(dt);
}

int main(int argc, char **argv) {
    window = init_window();

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    //glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glEnable(GL_DEPTH_TEST);

    // Enabling this will mess up skybox
    // glEnable(GL_CULL_FACE);

    init();
    GLfloat deltaTime;
    GLfloat lastFrame = 0.0f;
    const float tick_rate = 1.0f / 60.0f;
    float step = 0.0f;
    while (!glfwWindowShouldClose(window)) {
        GLfloat currentFrame = (GLfloat) glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();

        step += deltaTime;
        while (step >= tick_rate) {
            update(tick_rate);
            step -= tick_rate;
        }

        render(deltaTime);

		if (DebugUI::enabled) {
			DebugUI::render(deltaTime, world);
		}
		
        glfwSwapBuffers(window);
    }

	DebugUI::shutdown();
    glfwTerminate();
    delete world;

    return 0;
}

GLFWwindow *init_window() {
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    #ifndef NDEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    #endif

    GLFWwindow *window =
        glfwCreateWindow(WIDTH, HEIGHT, TITLE, nullptr, nullptr);

    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(window, GLFWHandler::key_callback);
    glfwSetCursorPosCallback(window, GLFWHandler::mouse_callback);
    glfwSetMouseButtonCallback(window, GLFWHandler::mousebtn_callback);
	glfwSetScrollCallback(window, GLFWHandler::scroll_callback);
	glfwSetCharCallback(window, GLFWHandler::char_callback);

    glfwSwapInterval(0);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        exit(EXIT_FAILURE);
    }

    #ifndef NDEBUG
        GLHelper::printGLInfo();
        GLHelper::registerDebugOutputCallback();
    #endif

    DebugUI::init(window);
    
    return window;
}

bool load_icon(GLFWimage *image, const char *path) {
    image->pixels = stbi_load(path, &image->width, &image->height, nullptr, 0);
    
    return image->pixels != nullptr;
}
