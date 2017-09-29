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
    return 20.0 + wave.amplitude * cos(theta * wave.frequency + t * wave.phase);
}

vec3 wave_normal(float x, float y, float t, Wave wave) {
    float theta = sqrt(dot(vec2(x, y), vec2(x, y)));
    float dx = wave.amplitude * wave.direction.x * wave.frequency * x
        * sin(theta * wave.frequency + t * wave.phase) / theta;
    float dy = wave.amplitude * wave.direction.y * wave.frequency * y
        * sin(theta * wave.frequency + t * wave.phase) / theta;
    
    return normalize(vec3(-dx, 1.0, -dy));
}

void main() {
    float height = single_wave(position.x, position.y, time, wave);
    pos = vec4(position.x, height, position.y, 1);
    normal = vec4(wave_normal(position.x, position.y, time, wave), 0);

    const float n_air = 1.0, n_water = 1.33;
    vec3 incident = normalize(vec3(pos) - light_pos);
    refracted_ray = vec4(refract(incident, vec3(normal), n_air / n_water), 0);
}