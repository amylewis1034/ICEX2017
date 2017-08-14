#version 330 core

in float dist_from_surface;

out vec4 color;

void main() {
    if (distance(vec2(0.5, 0.5), gl_PointCoord) > .5) {
        discard;
    }

    const float medium_attenuation = 0.13;
    color = vec4(1, 1, 1, 1) * exp(-medium_attenuation * dist_from_surface);
}
