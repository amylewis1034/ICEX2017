#ifndef DEBUGUI_HPP
#define DEBUGUI_HPP

#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class World;
struct GLFWwindow;

class DebugUI {
public:
	static void init(GLFWwindow *window);
	static void render(float dt, World *world);
	static void render2(float dt, World *world);
	static void shutdown();

	static bool enabled;

private:
	static void sendToFile(glm::vec3 pos, std::string name, glm::vec3 scale);	
};


#endif