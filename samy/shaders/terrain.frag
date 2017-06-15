#version 330 core

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexcoord;

uniform vec3 lightPos;
uniform vec3 lightInt;
uniform vec3 eye;

struct pointlight {
	vec3 pos, color;
	float a, b, c;
};

const int max_pointlights = 20;
uniform int num_pointlights;
uniform pointlight pointlights[max_pointlights];

uniform vec3 matAmb;
uniform vec3 matDif;
uniform vec3 matSpec;
uniform float matShine;
uniform float matAlpha;

uniform sampler2D texture0;

out vec4 color;

vec3 calcPointLight(pointlight light) {
	vec3 norm = normalize(fragNormal);
	vec3 lightvec = normalize(light.pos - fragPosition);
	vec3 view = normalize(eye - fragPosition);

	vec3 ambient = light.color * matAmb;
	vec3 diffuse = light.color * max(dot(norm, lightvec), 0) * matDif;

	vec3 h = normalize(0.5f * (view + lightvec));
	float vdotr = max(dot(norm, h), 0);
	vec3 specular = light.color * pow(vdotr, matShine) * matSpec;

	float dist = distance(light.pos, fragPosition);
	float attenuation = 1.0 / (light.a + light.b * dist + light.c * dist * dist);

	return attenuation * (ambient + diffuse + specular);
}

void main()
{
	vec3 norm = normalize(fragNormal);
	
	vec3 light = normalize(lightPos - fragPosition);
	vec3 view = normalize(eye - fragPosition);

    float tint = 0.2f;
    vec3 tcolor = texture(texture0, fragTexcoord / 10.0f).xyz;
    vec3 amb = mix(tcolor, matAmb, tint);
    vec3 dif = mix(tcolor, matDif, tint);
    vec3 spec = mix(tcolor, matSpec, tint);
    // vec3 amb = tcolor * matAmb;
    // vec3 dif = tcolor * matDif;
    // vec3 spec = tcolor * matSpec;

	vec3 ambient = lightInt * amb;
	vec3 diffuse = lightInt * max(dot(norm, light), 0) * dif;
	
	// Using Blinn-Torrance
	vec3 h = normalize(0.5f * (view + light));
	float vdotr = max(dot(norm, h), 0);
	
	vec3 specular = pow(vdotr, matShine) * lightInt * spec;


	color = vec4(ambient + diffuse + specular, .1);
	
	for (int i = 0; i < num_pointlights; i++)
		color += vec4(calcPointLight(pointlights[i]), 0);
}