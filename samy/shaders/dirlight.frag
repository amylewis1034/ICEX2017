#version 330 core

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexcoord;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpecular;

uniform vec3 eye;

uniform vec3 dirlightPos;
uniform vec3 dirlightColor;

uniform mat4 ls;
uniform sampler2D shadowMap;

out vec4 color;

float shadow(vec3 lsPosition) {
    vec3 shifted = (lsPosition + 1.0) * 0.5;

	float shade = 0.0;
    float bias = 0.01;
    float fragDepth = shifted.z - bias;

	if (fragDepth > 1.0) {
		return 0.0;
	}

	vec2 sampleOffset = 1.0 / textureSize(shadowMap, 0);

	if (fragDepth > texture(shadowMap, shifted.xy).r)
		shade += 0.2;
	if (fragDepth > texture(shadowMap, shifted.xy + vec2(sampleOffset.x, sampleOffset.y)).r)
		shade += 0.2;
	if (fragDepth > texture(shadowMap, shifted.xy + vec2(sampleOffset.x, -sampleOffset.y)).r)
		shade += 0.2;
	if (fragDepth > texture(shadowMap, shifted.xy + vec2(-sampleOffset.x, sampleOffset.y)).r)
		shade += 0.2;
	if (fragDepth > texture(shadowMap, shifted.xy + vec2(-sampleOffset.x, -sampleOffset.y)).r)
		shade += 0.2;

    return shade;
}

void main() {
    vec3 position = texture(gPosition, fragTexcoord).rgb;
    vec3 normal = texture(gNormal, fragTexcoord).rgb;
    vec4 albedo = texture(gAlbedoSpecular, fragTexcoord);
	
	vec3 light = normalize(dirlightPos - position);
	vec3 view = normalize(eye - position);

	vec3 ambient = dirlightColor * albedo.rgb;
	vec3 diffuse = dirlightColor * max(dot(normal, light), 0) * albedo.rgb;
	
	// Using Blinn-Torrance
	vec3 h = normalize(0.5 * (view + light));
	float vdotr = max(dot(normal, h), 0);
	
	vec3 specular = pow(vdotr, albedo.a) * dirlightColor * albedo.rgb;
    
    float shadowAmount = shadow((ls * vec4(position, 1.0)).xyz);
	color = vec4((1.0 - shadowAmount) * (ambient + diffuse + specular), 1);
}