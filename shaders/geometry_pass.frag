#version 330 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpecular;

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexcoord;

/* Albedo texture */
uniform sampler2D texture0;
/* Specular texture */
uniform sampler2D texture1;

void main() {
    gPosition = fragPosition;
    // gPosition = vec3(0.5f, 0.5f, 0.5f);
    gNormal = normalize(fragNormal);
    // gAlbedoSpecular = vec4(texture(texture0, fragTexcoord).xyz, texture(texture1, fragTexcoord).r);
    gAlbedoSpecular = vec4(texture(texture0, fragTexcoord).xyz, 1.0f);
    // gAlbedoSpecular = vec4(1.0f);
}