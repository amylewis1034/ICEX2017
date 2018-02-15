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

struct GerstnerWave {
    float q;
    float a;
    vec2 d;
    float omega;
    float phi;
};

uniform vec3 light_pos;
uniform float time;
uniform Wave wave;
uniform GerstnerWave waves[4];

// float single_wave(float x, float y, float t, Wave wave) {
//     float theta = sqrt(dot(vec2(x, y), vec2(x, y)));
//     return wave.amplitude * cos(theta * wave.frequency + t * wave.phase);
// }

// vec3 wave_gradient(float x, float y, float t, Wave wave) {
//     float theta = sqrt(dot(vec2(x, y), vec2(x, y)));
//     float dx = wave.amplitude * wave.direction.x * wave.frequency * x
//         * sin(theta * wave.frequency + t * wave.phase) / theta;
//     float dy = wave.amplitude * wave.direction.y * wave.frequency * y
//         * sin(theta * wave.frequency + t * wave.phase) / theta;
    
//     return vec3(-dx, 1.0, -dy);
// }

// float wave_height(float x, float y, float t) {
//     const float offset = 20.0;
//     float height = 0.0;
//     height += single_wave(x, y, t, wave); 
//     height += single_wave(x + 5, y + 5, t, wave); 
//     height += single_wave(x + 15, y, t, wave); 

//     return height + offset;
// }

// vec3 wave_normal(float x, float y, float t) {
//     vec3 n = vec3(0);
//     n += wave_gradient(x, y, t, wave);
//     n += wave_gradient(x + 5, y + 5, t, wave);
//     n += wave_gradient(x + 15, y, t, wave);

//     n.y = 1.0;
//     return normalize(n);
// }


vec3 gerstner_p(float x, float y, float t, GerstnerWave waves[4]) {
    vec3 result = vec3(x, y, 0);

    for (int i = 0; i < 4; i++) {
        result.x += waves[i].q * waves[i].a * waves[i].d.x
            * cos(dot(waves[i].omega * waves[i].d, vec2(x, y)) + waves[i].phi * t);
        result.y += waves[i].q * waves[i].a * waves[i].d.y
            * cos(dot(waves[i].omega * waves[i].d, vec2(x, y)) + waves[i].phi * t);
        result.z += waves[i].a
            * sin(dot(waves[i].omega * waves[i].d, vec2(x, y) + waves[i].phi * t));
    }

    return result;
}

vec3 gerstner_n(float x, float y, float t, vec2 P, GerstnerWave waves[4]) {
    vec3 result = vec3(0);

    for (int i = 0; i < 4; i++) {
        result.x += waves[i].d.x * waves[i].omega * waves[i].a
            * cos(waves[i].omega * dot(waves[i].d, P) + waves[i].phi * t);
        result.y += waves[i].d.y * waves[i].omega * waves[i].a
            * cos(waves[i].omega * dot(waves[i].d, P) + waves[i].phi * t);
        result.z += waves[i].q * waves[i].omega * waves[i].a
            * sin(waves[i].omega * dot(waves[i].d, P) + waves[i].phi * t);
    }

    return normalize(vec3(-result.x, -result.y, 1 - result.z));
}

void main() {
    float x = position.x, y = position.y, t = time;
    // GerstnerWave waves[] = GerstnerWave[4](
    //     GerstnerWave(0, 0.05, normalize(vec2(1, .2)), 1, 1),
    //     GerstnerWave(0, 0.35, normalize(vec2(1, -.1)), 3, 1),
    //     GerstnerWave(0, 0.5, normalize(vec2(1, 0)), 4, 1),
    //     GerstnerWave(0, 1.0, normalize(vec2(1, .3)), 8, 1)
    // );
    // for (int i = 0; i < 4; i++) waves[i].q = 0.2 / (waves[i].omega * waves[i].a * 4);
    vec3 P = gerstner_p(x, y, t, waves);
    vec3 N = gerstner_n(x, y, t, P.xy, waves);

    pos = vec4(P.xzy + vec3(0, 20, 0), 1);
    normal = vec4(N.xzy, 0);
    // pos = vec4(wave_height(position.x, position.y, time), 1);
    // normal = vec4(wave_normal(position.x, position.y, time), 0);

    const float n_air = 1.0, n_water = 1.33;
    vec3 incident = normalize(vec3(pos) - light_pos);
    refracted_ray = vec4(refract(incident, vec3(normal), n_air / n_water), 0);
}