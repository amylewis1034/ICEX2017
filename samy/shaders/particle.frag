#version 330 core

in vec3 fragPos;
in vec4 fragColor;

out vec4 color;

uniform vec3 eye;

void main() {
    vec2 temp = gl_PointCoord - vec2(0.5);
    float f = dot(temp, temp);

    if (f > 0.25) {
        discard;
    } 
    
    color = fragColor;
}
