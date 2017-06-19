#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cassert>
#include <iostream>
#include <cstdio>
#include <functional>
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include <icex_common.hpp>
#include <LevelParser.hpp>

using namespace std;
using namespace rapidjson;

typedef std::map<const std::string, Value *> SharedComponentMap;

static const std::string rpath {RESOURCE_PATH};
static const std::string spath {SHADER_PATH};

static glm::vec3 parseVec3(const Value &args, int offset) {
	assert(args.IsArray() && args.Size() >= offset + 3);

    return glm::vec3(args[offset].GetFloat(), 
            args[1 + offset].GetFloat(), 
            args[2 + offset].GetFloat());
}

static Component *parseTransform(Value &args) { 
    Transform *transform = new Transform();

    if (args.Size() >= 3)
        transform->translate(parseVec3(args, 0));
    if (args.Size() >= 7)
        transform->rotate(glm::degrees(args[3].GetFloat()), parseVec3(args, 4));
    if (args.Size() >= 10)
        transform->scale(parseVec3(args, 7));
        
    return (Component *) transform;
}

static Component *parseCamera(Value &args) {
    Camera *camera;

	if (args.Size() == 4) {
		camera = new Camera(
			args[0].GetFloat(),
			args[1].GetFloat(),
			args[2].GetFloat(),
			args[3].GetFloat()
		);
	}
	else {
		camera = new Camera();
		if (args.Size() == 1) {
			auto arg = args.GetArray()[0].GetString();
			if (!string(arg).compare(string("fps"))) {
				camera->setFirstPerson();
			}
		}
	}

    return (Component *) camera;
}

static Component *parseCollider(Value &args) {
	Collider *collider;

	if (args.Size() == 6) {
		collider = new Collider(
			parseVec3(args, 0),
			parseVec3(args, 3)
		);
	}
	else {
		collider = new Collider();
	}

	return (Component *) collider;
}

static Component *parseHeightmap(Value &args) {
    Heightmap *heightmap = new Heightmap();

	assert(args.Size() == 1 && args[0].IsString());
    heightmap->loadFromFile(rpath + args.GetArray()[0].GetString());

    return (Component *) heightmap;
}

static Component *parsePlayerInput(Value &args) {
    PlayerInput *playerInput = nullptr;

    if (args.Size() == 2) {
        playerInput = new PlayerInput(args[0].GetFloat(), args[1].GetFloat());
    }
    else {
        playerInput = new PlayerInput();
    }

    return (Component *) playerInput;
}

static Component *parsePRMInput(Value &args) {
    PRMInput *prmInput = nullptr;

	assert(args.Size() >= 2 && args[0].IsFloat() && args[1].IsString());
	if (args.Size() == 2) {
		prmInput = new PRMInput(args[0].GetFloat(), args[1].GetString());
	}
	else {
		prmInput = new PRMInput(args[0].GetFloat(), args[1].GetString(), args[2].GetBool());
	}

    return (Component *) prmInput;
}

static Component *parseMaterial(Value &args) {
	Material *material;

	if (args.Size() == 11) {
		material = new Material(
			parseVec3(args, 0),
			parseVec3(args, 3),
			parseVec3(args, 6),
			args[9].GetFloat(),
			args[10].GetFloat()
		);
	}
	else {
		material = new Material();
	}

	return (Component *)material;
}

static Component *parseMesh(Value &args) {
    Mesh *mesh;

    if (args.Size() == 1)
        mesh = new Mesh(rpath + args[0].GetString());

    return (Component *) mesh;
}

static Component *parseParticle(Value &args) {
	Particle *particle;

	if (args.Size() == 11) {
		particle = new Particle(args[0].GetFloat(), args[1].GetFloat(), args[2].GetFloat(),
								args[3].GetFloat(), args[4].GetFloat(), args[5].GetFloat(), 
								args[6].GetFloat(), args[7].GetFloat(), args[8].GetFloat(), 
								args[9].GetFloat(), args[10].GetFloat());
	}
	else {
		particle = new Particle(args[0].GetFloat(), args[1].GetFloat(), args[2].GetFloat(), 
								args[3].GetFloat(), args[4].GetFloat(), args[5].GetFloat(),
								args[6].GetFloat());
	}
	return (Component *) particle;
}

static Component *parseShader(Value &args) {
	Shader *shader;

	if (args.Size() == 2) {
		shader = new Shader(spath + args[0].GetString(), spath + args[1].GetString());
	}
	else if (args.Size() == 3) {
		shader = new Shader(spath + args[0].GetString(), spath + args[1].GetString(), spath + args[2].GetString());
	}
	else {
		shader = new Shader();
	}

	return (Component *)shader;
}

static Component *parseSkybox(Value &args) {
    Skybox *skybox;

    vector<string> skyboxNames{6};

    if (args.Size() == 6) {
        for (int i = 0; i < 6; i++) {
            string stringName = string(args[i].GetString(), args[i].GetStringLength());
            skyboxNames[i] = RESOURCE_PATH + stringName;
        }

        skybox = new Skybox(skyboxNames);
    }

    return (Component *) skybox;
}

static Component *parseTexture(Value &args) {
	Texture *texture;

	assert(args.Size() == 1);
	texture = new Texture(rpath + std::string(args[0].GetString(), args[0].GetStringLength()));

	return (Component *) texture;
}

// look up table of functions
const static std::map<std::string, std::function<Component *(Value &args)>> cmap = {
    {"Transform", parseTransform},
    {"Camera", parseCamera},
	{"Collider", parseCollider},
    {"Heightmap", parseHeightmap},
    {"PlayerInput", parsePlayerInput},
	{"PRMInput", parsePRMInput},
	{"Material", parseMaterial},
    {"Mesh", parseMesh},
	{"Particle", parseParticle},
	{"Shader", parseShader},
    {"Skybox", parseSkybox},
	{"Texture", parseTexture}
};

static Component *parseComponent(Value &c, const SharedComponentMap &sharedComponents) {
	string componentName = string(c["name"].GetString(), c["name"].GetStringLength());
	Component *component = nullptr;

	// cout << "\t" << componentName << endl;


	if (cmap.count(componentName) == 0) {
		if (sharedComponents.count(componentName) == 0) {
			// cout << "\t\t" << "Unknown component, ignoring" << endl;
			return nullptr;
		}
		else {
			Value *c = sharedComponents.at(componentName);
			std::string name {(*c)["name"].GetString(), (*c)["name"].GetStringLength()};
			Value &args = (*c)["args"];

			if (cmap.count(name) == 0) {
				return nullptr;
			}
			component = cmap.at(name)(args);
		}
	}
	else {
		assert(c.HasMember("args") && c["args"].IsArray());

		component = cmap.at(componentName)(c["args"]);
	}

	return component;
}

static GameObject *parseGameObject(Value &g, const SharedComponentMap &sharedComponents) {
	GameObject *gameObject;

	if (g.HasMember("tag")) {
		gameObject = new GameObject(g["tag"].GetString());
	}
	else {
		gameObject = new GameObject();
	}

	assert(g.HasMember("components") && g["components"].IsArray());

	for (auto& c : g["components"].GetArray()) {
		Component *component = parseComponent(c, sharedComponents);

		if (component != nullptr) {
			gameObject->addComponent(component);
		}
	}

	return gameObject;
}

static Document jsonFromFile(const std::string &filename) {
	FILE* fp = fopen(filename.c_str(), "r"); // non-windows use "r", windows use "rb"
	assert(fp != nullptr);
	char readBuffer[65536];
	FileReadStream is(fp, readBuffer, sizeof(readBuffer));

	Document document;
	if (document.ParseStream<kParseCommentsFlag>(is).HasParseError())
		throw std::invalid_argument("json parse error");

	fclose(fp);

	return document;
}

void LevelParser::ParseWorld(World *world, const std::string &filename) {
	Document document = jsonFromFile(filename);

    assert(document.IsObject());

	SharedComponentMap sharedComponents;
	if (document.HasMember("SharedComponents")) {
		assert(document["SharedComponents"].IsObject());

		for (auto &c : document["SharedComponents"].GetObject()) {
			std::string name {c.name.GetString(), c.name.GetStringLength()};
			// cout << name <<  endl;

			sharedComponents[name] = &c.value;
		}
	}

    assert(document.HasMember("GameObjects") && document["GameObjects"].IsArray());

    // Goes through all game objects within JSON
    for (auto& g : document["GameObjects"].GetArray()) {    
        // std::cout << g["tag"].GetString() << std::endl;
        
		GameObject *gameObject = parseGameObject(g, sharedComponents);

        world->addGameObject(gameObject);
    }

	if (document.HasMember("MainLight")) {
		assert(document["MainLight"].IsObject());

		auto &light = document["MainLight"];

		world->setMainLight(
			parseVec3(light["pos"].GetArray(), 0),
			parseVec3(light["color"].GetArray(), 0)
		);
	}

	if (document.HasMember("PointLights")) {
		assert(document["PointLights"].IsArray());

		for (auto &light : document["PointLights"].GetArray()) {
			assert(light.IsObject() && light.HasMember("pos") && light.HasMember("color"));
			assert(light.HasMember("a") && light.HasMember("b") && light.HasMember("c"));

			world->addPointLight(
				parseVec3(light["pos"].GetArray(), 0),
				parseVec3(light["color"].GetArray(), 0),
				light["a"].GetFloat(),
				light["b"].GetFloat(),
				light["c"].GetFloat()
			);
		}
	}
}