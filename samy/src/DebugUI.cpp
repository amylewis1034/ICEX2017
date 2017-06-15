#include <DebugUI.hpp>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

#include <World.hpp>
#include <GameObject.hpp>
#include <Renderers/Renderer.hpp>
#include <Renderers/DeferredRenderer.hpp>
#include <Renderers/DeferredShadowRenderer.hpp>
#include <Renderers/PostprocessRenderer.hpp>
#include <Components/PlayerInput.hpp>
#include <Components/Camera.hpp>
#include <LevelParser.hpp>
#include <Graphics/GLFramebuffer.hpp>
#include <memory>

#include <icex_common.hpp>

bool DebugUI::enabled = false;

void DebugUI::init(GLFWwindow *window) {
	ImGui_ImplGlfwGL3_Init(window, false);
}

void DebugUI::render(float dt, World *world) {
	ImGui_ImplGlfwGL3_NewFrame();

	/* Display frames per second */
	ImGui::Text("FPS: %.3f", 1.0f / dt);

	/* Display current position and forward */
	{
		GameObject *player = world->getGameObjectWithComponent<PlayerInput>();
		if (player != nullptr) {
			Transform *t = player->getComponent<Transform>();
			const glm::vec3 &pos = t->getPosition();
			const glm::vec3 &f = t->getForward();
			ImGui::Text("Position: %f %f %f", pos.x, pos.y, pos.z);
			ImGui::Text("Forward: %f %f %f", f.x, f.y, f.z);
		}
	}

	/* Play with lights */
	{
		float scale = 1.5f;
		if (ImGui::Button("Brighten")) {
			world->scaleLightIntensity(scale);
		}
		if (ImGui::Button("Darken")) {
			world->scaleLightIntensity(1.0f / scale);
		}
	}

	/* Adjust postprocessing values */
	{
		PostprocessRenderer *pr = dynamic_cast<PostprocessRenderer *>(world->getPostrenderer());
		float gamma = pr->getGamma(), exposure = pr->getExposure(), fogDensity = pr->getFogDensity(), 
			ten = pr->getTen(), factor1 = pr->getFactor1(), factor2 = pr->getFactor2();
		ImGui::SliderFloat("Gamma", &gamma, 0.0f, 5.0f);
		ImGui::SliderFloat("Exposure", &exposure, 0.0f, 10.0f);
		ImGui::SliderFloat("Fog Density", &fogDensity, 0.0f, 100.0f);
		ImGui::SliderFloat("ten", &ten, 0.0f, 20.0f);
		ImGui::SliderFloat("factor1", &factor1, 0.001f, 0.01f);
		ImGui::SliderFloat("factor2", &factor2, 0.001f, 0.01f);
		pr->setGamma(gamma);
		pr->setExposure(exposure);
		pr->setFogDensity(fogDensity);
		pr->setTen(ten);
		pr->setFactor1(factor1);
		pr->setFactor2(factor2);
	}

	/* Toggle free camera */
	if (ImGui::Button("Toggle free camera")) {
		GameObject *player = world->getGameObjectWithComponent<PlayerInput>();
		if (player != nullptr) {
			player->getComponent<PlayerInput>()->toggleHeightLock();
		}
	}

	/* Adjust camera offset and spring factor */
	{
		GameObject *camObject = world->getGameObjectWithComponent<Camera>();
		if (camObject != nullptr) {
			Camera *camera = camObject->getComponent<Camera>();

			static glm::vec3 offset = camera->getOffset();
			ImGui::SliderFloat3("Camera offset", glm::value_ptr(offset), 0.0f, 5.0f);
			camera->setOffset(offset);

			static float k = camera->getSpringFactor();
			ImGui::SliderFloat("Camera spring factor", &k, 5.0f, 20.0f);
			camera->setSpringFactor(k);

			if (ImGui::Button("Set first person")) {
				camera->setFirstPerson();
				offset = camera->getOffset();
				k = camera->getSpringFactor();
			}
		}
	}

	/* Modify player speed and sensitivity */
	{
		GameObject *player = world->getGameObjectWithComponent<PlayerInput>();
		if (player != nullptr) {
			PlayerInput *pi = player->getComponent<PlayerInput>();

			static float speed = pi->getSpeed(), sensitivity = pi->getSensitivity();
			ImGui::DragFloat("Player speed", &speed, 0.1f, 0.0f, 50.0f);
			ImGui::DragFloat("Player sensitivity", &sensitivity, 0.0005f, 0.0f, 0.5f, "%.4f");

			pi->setSpeed(speed);
			pi->setSensitivity(sensitivity);
		}
	}

	/* Show gBuffer */
	{
		ImVec2 image_size = ImVec2(200, 100);
		ImVec4 border_color = ImVec4(255, 255, 255, 255);
		ImVec2 uv0 = ImVec2(0, 1), uv1 = ImVec2(1, 0);
		ImVec4 tint = ImColor(255, 255, 255);

		const DeferredRenderer *dr = dynamic_cast<const DeferredRenderer *>(world->getRenderer());
		if (dr != nullptr) {
			ImGui::Text("gBuffer");
			/* Positions */
			ImTextureID tex_id = (ImTextureID)(uintptr_t)dr->getPosition();
			ImGui::Image(tex_id, image_size, uv0, uv1, tint, border_color);
			/* Normals */
			tex_id = (ImTextureID)(uintptr_t)dr->getNormal();
			ImGui::Image(tex_id, image_size, uv0, uv1, tint, border_color);
			/* Albedo + Specular */
			tex_id = (ImTextureID)(uintptr_t)dr->getAlbedoSpecular();
			ImGui::Image(tex_id, image_size, uv0, uv1, tint, border_color);
		}
	}

	/* Show light depth */
	{
		ImVec2 image_size = ImVec2(200, 100);
		ImVec4 border_color = ImVec4(255, 255, 255, 255);
		ImVec2 uv0 = ImVec2(0, 1), uv1 = ImVec2(1, 0);
		ImVec4 tint = ImColor(255, 255, 255);

		const DeferredShadowRenderer *dr = dynamic_cast<const DeferredShadowRenderer *>(world->getRenderer());
		if (dr != nullptr) {
			ImGui::Text("gBuffer");
			/* Positions */
			ImTextureID tex_id = (ImTextureID)(uintptr_t)dr->getPosition();
			ImGui::Image(tex_id, image_size, uv0, uv1, tint, border_color);
			/* Normals */
			tex_id = (ImTextureID)(uintptr_t)dr->getNormal();
			ImGui::Image(tex_id, image_size, uv0, uv1, tint, border_color);
			/* Albedo + Specular */
			tex_id = (ImTextureID)(uintptr_t)dr->getAlbedoSpecular();
			ImGui::Image(tex_id, image_size, uv0, uv1, tint, border_color);

			/* Light Depth */
			tex_id = (ImTextureID)(uintptr_t)dr->getLightDepth();
			ImGui::Image(tex_id, image_size, uv0, uv1, tint, border_color);
		}
	}

	/* Show the ImGui test window */
	{
		static bool showTestWindow = false;
		if (ImGui::Button("Show test window")) {
			showTestWindow = !showTestWindow;
		}
		if (showTestWindow) {
			ImGui::ShowTestWindow();
		}
	}

	ImGui::Render();
}

void DebugUI::shutdown() {
	ImGui_ImplGlfwGL3_Shutdown();
}
