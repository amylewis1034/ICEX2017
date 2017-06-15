#ifndef WORLD_HPP
#define WORLD_HPP

#include <GameObject.hpp>

#include <Components/Mesh.hpp>
#include <Components/Shader.hpp>
#include <Components/Material.hpp>
#include <Graphics/GLShaderProgram.hpp>
#include <Components/Texture.hpp>

#include "QuadTree/Node.hpp"
#include "QuadTree/ViewFrustrum.hpp"
#include <Renderers/Renderer.hpp>

class World {
public:
    World();
    ~World();

    void init();
    void update(float dt);
    void render(float dt);
	void render(const glm::mat4 &projection, const glm::mat4 &view, const glm::vec3 &eye);

    void addGameObject(GameObject *gameobject);
    void removeGameObject(GameObject *gameobject);

	GameObject *getGameObjectByTag(const std::string &tag) const;
	std::vector<GameObject *> getGameObjectsByTag(const std::string &tag) const;

	template <typename T>
	GameObject *getGameObjectWithComponent() const {
		for (GameObject *g : gameobjects) {
			if (g->hasComponent<T>()) {
				return g;
			}
		}

		return nullptr;
	}

	template <typename T>
	std::vector<GameObject *> getGameObjectsWithComponent() const {
		std::vector<GameObject *> gs;

		for (GameObject *g : gameobjects) {
			if (g->hasComponent<T>()) {
				gs.push_back(g);
			}
		}

		return gs;
	}

	void setMainLight(glm::vec3 pos, glm::vec3 color);
	void addPointLight(glm::vec3 pos, glm::vec3 color, float a, float b, float c);
	void scaleLightIntensity(float scale);
	void setLightUniforms(GLShaderProgram &program) const;
	void setLightUBO(GLShaderProgram &program) const;
	const glm::vec3 &getMainlightPosition() const;
	const glm::vec3 &getMainlightColor() const;

	
	const Renderer *getRenderer() const;
	Renderer *getPostrenderer() const;
	const std::vector<GameObject *> getRenderables(const glm::mat4 &projection, const glm::mat4 &view);

private:
    std::vector<GameObject *> gameobjects;
    // std::vector<GameObject *> renderable;
    std::vector<GameObject*> renderables;
	std::vector<GameObject *> collidable;
    std::vector<GameObject *> textured;

	Renderer *renderer, *postrenderer;

    GameObject *camera;

    std::vector<GameObject *> deleteQueue;

    GLShaderProgram program;
    Shader *defaultShader;
    Material *defaultMaterial;
	Texture *defaultTexture;

    Node quadTreeRenderable; // for view frustrum, will have ltrees
                        // meshes, and collidables?
    Node quadTreeCollidables; // for collision: will only have 
                                  // collidable stuff
    ViewFrustrum frustrum;

    GameObject *heightMapComponent;
};

#endif