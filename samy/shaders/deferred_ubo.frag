/* Deferred shading pass. Uses gBuffer textures and outputs to a single quad. */
#version 330 core

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexcoord;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpecular;

uniform vec3 eye;

struct pointlight {
	vec3 pos, color;
	float a, b, c;
};

const int max_pointlights = 1024;
layout (std140) uniform Lights {
	vec3 lightPos;
	vec3 lightInt;
	int num_pointlights;
	pointlight pointlights[max_pointlights];
};

out vec4 color;

vec3 calcPointLight(pointlight light, vec3 position, vec3 normal, vec4 albedo) {
	vec3 lightvec = normalize(light.pos - position);
	vec3 view = normalize(eye - position);

	vec3 ambient = light.color * albedo.rgb;
	vec3 diffuse = light.color * max(dot(normal, lightvec), 0) * albedo.rgb;

	vec3 h = normalize(0.5f * (view + lightvec));
	float vdotr = max(dot(normal, h), 0);
	vec3 specular = light.color * pow(vdotr, albedo.a) * albedo.rgb;

	float dist = distance(light.pos, position);
	float attenuation = 1.0 / (light.a + light.b * dist + light.c * dist * dist);

	return attenuation * (ambient + diffuse + specular);
}

void main()
{
	vec3 position = texture(gPosition, fragTexcoord).rgb;
    vec3 normal = texture(gNormal, fragTexcoord).rgb;
    vec4 albedo = texture(gAlbedoSpecular, fragTexcoord);
	
	vec3 light = normalize(lightPos - position);
	vec3 view = normalize(eye - position);

	vec3 ambient = lightInt * albedo.rgb;
	vec3 diffuse = lightInt * max(dot(normal, light), 0) * albedo.rgb;
	
	// Using Blinn-Torrance
	vec3 h = normalize(0.5f * (view + light));
	float vdotr = max(dot(normal, h), 0);
	
	vec3 specular = pow(vdotr, albedo.a) * lightInt * albedo.rgb;


	color = vec4(ambient + diffuse + specular, 1.0f);
	
	for (int i = 0; i < num_pointlights; i++)
		color += vec4(calcPointLight(pointlights[i], position, normal, albedo), 0);
}