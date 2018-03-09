#ifndef GLOW_HPP
#define GLOW_HPP

#include <World.hpp>
#include <GameObject.hpp>

#include <Components/Camera.hpp>
#include <Components/Collider.hpp>
#include <Components/Component.hpp>
#include <Components/FirstPersonCamera.hpp>
#include <Components/Heightmap.hpp>
#include <Components/Texture.hpp>
#include <Components/Material.hpp>
#include <Components/Mesh.hpp>
#include <Components/Particle.hpp>
#include <Components/PlayerInput.hpp>
#include <Components/PRMInput.hpp>
#include <Components/PRMCombo.hpp>
#include <Components/PRMNorms.hpp>
#include <Components/PRMThirds.hpp>
#include <Components/ProjectiveTexture.hpp>
#include <Components/Shader.hpp>
#include <Components/Skybox.hpp>
#include <Components/Transform.hpp>
#include <Components/WaterMesh.hpp>

#include <Input/GLFWHandler.hpp>
#include <Input/Keyboard.hpp>
#include <Input/Mouse.hpp>

#include "QuadTree/Node.hpp"
#include "QuadTree/Rectangle.hpp"
#include "QuadTree/ViewFrustrum.hpp"


#include <vector>
#include <string>
#include <random>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/* Game configuration */
const int WIDTH = 1280, HEIGHT = 720;

#define TITLE "ICEX 2017"

#ifndef SHADER_PATH
#define SHADER_PATH "../shaders/"
#endif

#ifndef RESOURCE_PATH
#define RESOURCE_PATH "../resources/"
#endif

const float WORLD_SIZE = 50.0f;

/* Player configuration */
const float PLAYER_SPEED = 2.0f;
const float PLAYER_SENSITIVITY = 0.001f;

static float randFloat(float l, float h) {
    float r = rand() / (float)RAND_MAX;
    return (1.0f - r) * l + r * h;
}

#endif
