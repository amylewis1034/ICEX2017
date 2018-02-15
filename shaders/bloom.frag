#version 330 core

out vec4 bloomBright;

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexcoord;

uniform sampler2D color_in;

void main() {
    vec4 color = texture(color_in, fragTexcoord);
    // magic number from learnopengl.com, still don't know where it comes from but just a weighting
    float brightness = dot(color.xyz, vec3(0.2126, 0.7162, 0.0722));
    if (brightness > 1.0) {
        bloomBright = color;
    }
    else {
        bloomBright = vec4(0, 0, 0, 1);
    }
}
