#version 330 core

in vec3 fragPosition;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpecular;

uniform vec2 screenSize;
uniform vec3 eye;

struct pointlight {
	vec3 pos, color;
	float a, b, c;
};

uniform pointlight plight;

out vec4 color;

vec3 calcPointLight(pointlight light, vec3 position, vec3 normal, vec4 albedo) {
	vec3 lightvec = normalize(light.pos - position);
	vec3 view = normalize(eye - position);

	vec3 ambient = light.color * albedo.rgb;
	vec3 diffuse = light.color * max(dot(normal, lightvec), 0) * albedo.rgb;

	vec3 h = normalize(0.5 * (view + lightvec));
	float vdotr = max(dot(normal, h), 0);
	vec3 specular = light.color * pow(vdotr, albedo.a) * albedo.rgb;

	float dist = distance(light.pos, position);
	float attenuation = 1.0 / (light.a + light.b * dist + light.c * dist * dist);

	return attenuation * (ambient + diffuse + specular);
}

void main() {
    vec2 fragTexcoord = gl_FragCoord.xy / screenSize;

    vec3 position = texture(gPosition, fragTexcoord).rgb;
    vec3 normal = texture(gNormal, fragTexcoord).rgb;
    vec4 albedo = texture(gAlbedoSpecular, fragTexcoord);

    color = vec4(calcPointLight(plight, position, normal, albedo), 1);
    // color = vec4(1, 0, 0, 1);
}