#include <World.hpp>
#include <LevelParser.hpp>
#include "GLFWHandler.hpp"

#include "imgui/imgui_impl_glfw_gl3.h"

#include <DebugUI.hpp>

extern World *world;

void GLFWHandler::key_callback(GLFWwindow *window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
	else if (key == GLFW_KEY_GRAVE_ACCENT && action == GLFW_PRESS) {
		DebugUI::enabled = !DebugUI::enabled;
		glfwSetInputMode(window, GLFW_CURSOR, DebugUI::enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
	}
    else if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        delete world;
        world = new World();
        
        world->init();
    }
	else if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS) {
		int mode = glfwGetInputMode(window, GLFW_CURSOR);
		glfwSetInputMode(window, GLFW_CURSOR, (mode == GLFW_CURSOR_DISABLED) ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
	}

    Keyboard::setKeyStatus(key, action);

	ImGui_ImplGlfwGL3_KeyCallback(window, key, scancode, action, mode);
}

void GLFWHandler::mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    static bool firstMouse = true;

    if (firstMouse) {
        Mouse::setMousePos(xpos, ypos);
        Mouse::update();
        firstMouse = false;
    }

    Mouse::setMousePos(xpos, ypos);
}

void GLFWHandler::mousebtn_callback(GLFWwindow *window, int button, int action, int mods) {
    Mouse::setMouseButton(button, action);

	ImGui_ImplGlfwGL3_MouseButtonCallback(window, button, action, mods);
}

void GLFWHandler::scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
	ImGui_ImplGlfwGL3_ScrollCallback(window, xoffset, yoffset);
}

void GLFWHandler::char_callback(GLFWwindow *window, unsigned int codepoint) {
	ImGui_ImplGlfwGL3_CharCallback(window, codepoint);
}
