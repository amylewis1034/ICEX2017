#version 330 core

layout (location = 0) in vec2 position;

out data {
    vec4 pos;
    vec4 normal;
    vec4 refracted_ray;
};

struct Wave {
    float amplitude, frequency, phase;
    vec2 direction;
};

uniform vec3 light_pos;
uniform float time;
uniform Wave wave;

float single_wave(float x, float y, float t, Wave wave) {
    float theta = sqrt(dot(vec2(x, y), vec2(x, y)));
    return wave.amplitude * cos(theta * wave.frequency + t * wave.phase);
}

vec3 wave_gradient(float x, float y, float t, Wave wave) {
    float theta = sqrt(dot(vec2(x, y), vec2(x, y)));
    float dx = wave.amplitude * wave.direction.x * wave.frequency * x
        * sin(theta * wave.frequency + t * wave.phase) / theta;
    float dy = wave.amplitude * wave.direction.y * wave.frequency * y
        * sin(theta * wave.frequency + t * wave.phase) / theta;
    
    return vec3(-dx, 1.0, -dy);
}

float wave_height(float x, float y, float t) {
    const float offset = 20.0;
    float height = 0.0;
    height += single_wave(x, y, t, wave); 
    height += single_wave(x + 5, y + 5, t, wave); 
    height += single_wave(x + 15, y, t, wave); 

    return height + offset;
}

vec3 wave_normal(float x, float y, float t) {
    vec3 n = vec3(0);
    n += wave_gradient(x, y, t, wave);
    n += wave_gradient(x + 5, y + 5, t, wave);
    n += wave_gradient(x + 15, y, t, wave);

    n.y = 1.0;
    return normalize(n);
}

void main() {
    float height = wave_height(position.x, position.y, time);
    pos = vec4(position.x, height, position.y, 1);
    normal = vec4(wave_normal(position.x, position.y, time), 0);

    const float n_air = 1.0, n_water = 1.33;
    vec3 incident = normalize(vec3(pos) - light_pos);
    refracted_ray = vec4(refract(incident, vec3(normal), n_air / n_water), 0);
}