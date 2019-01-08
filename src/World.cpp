#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <World.hpp>
#include <GameObject.hpp>
#include <Renderers/Renderer.hpp>
#include <Renderers/DeferredRenderer.hpp>
#include <Renderers/DeferredShadowRenderer.hpp>
#include <Renderers/ForwardRenderer.hpp>
#include <Renderers/PostprocessRenderer.hpp>
#include <Renderers/ParticleRenderer.hpp>

#include <Components/Heightmap.hpp>
#include <Components/Skybox.hpp>
#include <Components/Transform.hpp>
#include <Components/Mesh.hpp>
#include <Components/Camera.hpp>
#include <Components/Shader.hpp>
#include <Components/Particle.hpp>
#include <Components/PlayerInput.hpp>
#include <Components/Material.hpp>
#include <Components/Texture.hpp>
#include <Graphics/GLShaderProgram.hpp>
#include <Graphics/GLQuad.hpp>
#include <Graphics/GLFramebuffer.hpp>
#include <Graphics/GLBuffer.hpp>

#include <QuadTree/Node.hpp>
#include <QuadTree/Rectangle.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <time.h>

#include <icex_common.hpp>

extern GLFWwindow *window;

struct pointlight {
	glm::vec3 pos, color;
	float a, b, c;
};
struct directionallight {
	glm::vec3 pos, color;
};

std::vector<pointlight> pointlights{ 1 };
static directionallight mainlight;
static bool pointlight_dirty = true;

static void removeGameObjectFromVector(GameObject *g, std::vector<GameObject *> &v) {
    auto toremove = std::find(v.begin(), v.end(), g);
    if (toremove != v.end()) {
        v.erase(toremove);
    }
}

World::World() {}

World::~World() {
    pointlights.clear();

    for (GameObject *g : gameobjects) {
        delete g;
    }

	delete renderer;
	delete postrenderer;
	delete particlerenderer;
}

void World::init() {
	srand(time(NULL));
    for (GameObject *g : gameobjects) {
        g->init();
    }

	for (GameObject *g : gameobjects) {
		if (g->hasComponent<Camera>()) {
			this->camera = g;
			break;
		}
	}

	GameObject *terrain = this->getGameObjectWithComponent<Heightmap>();
	if (terrain != nullptr) {
		Heightmap *h = terrain->getComponent<Heightmap>();

		for (GameObject *g : gameobjects) {
			if (g->getTag() == "orb" || g->getTag() == "rock") {
				Transform *t = g->getComponent<Transform>();

				float offset = h->getHeightAt(t->getPosition().x, t->getPosition().z);
				if (g->getTag() == "orb") {
					offset += 1.0f;

					glm::vec3 color {2.0f};
					glm::vec3 pos = t->getPosition() + glm::vec3(0.0f, offset, 0.0f);
					this->addPointLight(pos, color, 1.0f, 0.7f, 1.8f);
				}

				t->translate(glm::vec3(0.0f, offset, 0.0f));
			}
		}
	}

	pointlights[0] = {
		glm::vec3(0.0f), glm::vec3(0.6f, 0.75f, 0.6f),
		1.0f, 0.7f, 1.8f
	};

	auto randf = []() {return rand() / (float)RAND_MAX;};

    defaultMaterial = new Material();
	defaultTexture = new Texture(RESOURCE_PATH "textures/default.png");
    defaultShader = new Shader(SHADER_PATH "phong.vert", SHADER_PATH "phong.frag");
    program = defaultShader->getProgram();

	// renderer = new ForwardRenderer();
	// renderer = new DeferredRenderer();
	renderer = new DeferredShadowRenderer();

	postrenderer = new PostprocessRenderer();
	// postrenderer = nullptr;

	particlerenderer = new ParticleRenderer();
}

void World::update(float dt) {
    static float gametime = 0.0f;
    gametime += dt;

    GameObject *player = this->getGameObjectWithComponent<Camera>();
	
	if (Keyboard::getKeyToggle(GLFW_KEY_P)) {
		GameObject *tmp = this->getGameObjectWithComponent<PRMInput>();
		if (tmp) {
			player = tmp;
		}
	}

	camera = player;

    Transform *tPlayer = player->getComponent<Transform>();
    glm::vec3 playerPos = tPlayer->getPosition();

	for (GameObject *g : gameobjects) {
		g->update(dt);
	}

    for (GameObject *g : deleteQueue) {
        removeGameObjectFromVector(g, gameobjects);
        removeGameObjectFromVector(g, renderables);
    }
}

void World::render(float dt) {
	static float gametime = 0.0f;
	static float transition = 0.0f;
	const float ttime = 2.0f;
	gametime += dt;

	Camera *cam = camera->getComponent<Camera>();
	Transform *camPosTransform = camera->getComponent<Transform>();

    glm::mat4 projection = cam->getProjection();
    glm::mat4 view = cam->getView();
    
    glm::vec3 eye = camPosTransform->getPosition();

	this->render(projection, view, eye);

	for (GameObject *g : gameobjects) {
		g->postrender(projection, view);
	}
}

void World::render(const glm::mat4 &projection, const glm::mat4 &view, const glm::vec3 &eye) {
    renderer->render(projection, view, eye, *this);
	particlerenderer->render(projection, view, eye, *this);
    postrenderer->render(projection, view, eye, *this);
}

void World::addGameObject(GameObject *gameobject) {
    gameobjects.push_back(gameobject);

    // if (gameobject->hasComponent<Mesh>() || gameobject->hasComponent<Heightmap>() || gameobject->hasComponent<WaterMesh>()) {
    //     renderables.push_back(gameobject);
    // }

	if (gameobject->hasComponent<Mesh>() || gameobject->hasComponent<Heightmap>()) {
        renderables.push_back(gameobject);
    }
	
	if (gameobject->hasComponent<Particle>()) {
		particleSystems.push_back(gameobject);
	}
}

void World::removeGameObject(GameObject *gameobject) {
    deleteQueue.push_back(gameobject);
}

GameObject *World::getGameObjectByTag(const std::string &tag) const {
	for (GameObject *g : gameobjects) {
		if (g->getTag() == tag) {
			return g;
		}
	}

	return nullptr;
}

std::vector<GameObject *> World::getGameObjectsByTag(const std::string &tag) const {
	std::vector<GameObject *> gs;

	for (GameObject *g : gameobjects) {
		if (g->getTag() == tag) {
			gs.push_back(g);
		}
	}

	return gs;
}

void World::setMainLight(glm::vec3 pos, glm::vec3 color) {
	mainlight = { pos, color };
}

void World::setMainLightColor(const glm::vec3 &color) {
	mainlight.color = color;
}

void World::addPointLight(glm::vec3 pos, glm::vec3 color, float a, float b, float c) {
	pointlight p = { pos, color, a, b, c };

	pointlights.push_back(p);
}

void World::addRenderSetting(RenderSettings renderSettings) {
	this->renderSettings = renderSettings;
}

void World::scaleLightIntensity(float scale) {
	for (pointlight &p : pointlights) {
		p.color *= scale;
		p.color = glm::clamp(p.color, glm::vec3(0.0f), glm::vec3(15.0f));
	}
}

void World::setLightUniforms(GLShaderProgram &program) const {
	glUniform3fv(program.uniformLocation("lightPos"), 1, glm::value_ptr(mainlight.pos));
	glUniform3fv(program.uniformLocation("lightInt"), 1, glm::value_ptr(mainlight.color));

	glUniform1i(program.uniformLocation("num_pointlights"), pointlights.size());
	for (size_t i = 0; i < pointlights.size(); i++) {
		std::stringstream out;
		out << i;
		std::string name = std::string("pointlights[") + out.str() + std::string("]");

		glUniform3fv(program.uniformLocation((name + std::string(".pos")).c_str()), 1, glm::value_ptr(pointlights[i].pos));

		glUniform3fv(program.uniformLocation((name + std::string(".color")).c_str()), 1, glm::value_ptr(pointlights[i].color));
		glUniform1f(program.uniformLocation((name + std::string(".a")).c_str()), pointlights[i].a);
		glUniform1f(program.uniformLocation((name + std::string(".b")).c_str()), pointlights[i].b);
		glUniform1f(program.uniformLocation((name + std::string(".c")).c_str()), pointlights[i].c);
	}
}

void World::setLightUBO(GLShaderProgram &program) const {
	static GLuint ubo = 0;

	if (ubo == 0) {
		glGenBuffers(1, &ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo);

		/* TODO: this should be variable */
		GLsizeiptr size = 32 + pointlights.size() * 48;
		glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);

		pointlight_dirty = true;
	}

	/* TODO: - move lights to a separate component/system
	 * 		 - keep lights sorted by whether dirty or not
	 * 	     - when light becomes dirty, swap it with first clean in array (so first n lights are dirty)
	 */
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	/* HACK: always update first light (the one on player) */
	glBufferSubData(GL_UNIFORM_BUFFER, 32, sizeof(glm::vec3), glm::value_ptr(pointlights[0].pos));
	glBufferSubData(GL_UNIFORM_BUFFER, 48, sizeof(glm::vec3), glm::value_ptr(pointlights[0].color));
	
	if (pointlight_dirty) {
		glBufferSubData(GL_UNIFORM_BUFFER, 0, 12, glm::value_ptr(mainlight.pos));
		glBufferSubData(GL_UNIFORM_BUFFER, 16, 12, glm::value_ptr(mainlight.color));
		GLint num_pointlights = pointlights.size();
		glBufferSubData(GL_UNIFORM_BUFFER, 28, 4, &num_pointlights);

		GLintptr offset = 32;
		for (int i = 0; i < pointlights.size(); i++) {
				glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(glm::vec3), glm::value_ptr(pointlights[i].pos));
				glBufferSubData(GL_UNIFORM_BUFFER, offset + 16, sizeof(glm::vec3), glm::value_ptr(pointlights[i].color));

				glBufferSubData(GL_UNIFORM_BUFFER, offset + 28, sizeof(float), &pointlights[i].a);
				glBufferSubData(GL_UNIFORM_BUFFER, offset + 32, sizeof(float), &pointlights[i].b);
				glBufferSubData(GL_UNIFORM_BUFFER, offset + 36, sizeof(float), &pointlights[i].c);

				offset += 48;
		}
		pointlight_dirty = false;
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);


	GLuint index = glGetUniformBlockIndex(program.getHandle(), "Lights");
	glUniformBlockBinding(program.getHandle(), index, 0);
}

const glm::vec3 &World::getMainlightPosition() const {
	return mainlight.pos;
}

const glm::vec3 &World::getMainlightColor() const {
	return mainlight.color;
}

const Renderer *World::getRenderer() const {
	return this->renderer;
}

Renderer *World::getPostrenderer() const {
	return this->postrenderer;
}

const std::vector<GameObject *> World::getRenderables(const glm::mat4 &projection, const glm::mat4 &view) {
	return renderables;
}

const std::vector<GameObject *> World::getParticleSystems() {
	return particleSystems;
}

const RenderSettings &World::getRenderSetting() const {
	return renderSettings;
}

void World::setKatieRenderSetting(bool setting) {
	renderSettings.isKatie = setting;
}

const float World::getNear() const {
	assert(camera && camera->getComponent<Camera>());

	return camera->getComponent<Camera>()->getNear();
}

const float World::getFar() const {
	assert(camera && camera->getComponent<Camera>());
	
	return camera->getComponent<Camera>()->getFar();
}
