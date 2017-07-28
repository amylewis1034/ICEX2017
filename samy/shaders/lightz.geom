#version 330 core

layout (points) in;
layout (points, max_vertices = 1) out;

in vec3 geomPos[];

uniform mat4 projection; // light space projection matrix
uniform mat4 view;  // light space matrix
uniform vec3 lightPos;
uniform float time;
uniform sampler2D shadowMap;

// out vec3 fragPos;

float surface(float x, float y) {
    return cos(sqrt(x * x + y * y));
}

vec3 surface_gradient(vec3 point) {
    float x = point.x, y = point.y, z = point.z;
    float groot = sqrt(x*x + y*y);
    float singroot = sin(groot);

    return vec3(x*singroot / groot, y * singroot / groot, 1);
}

float objective_function(vec3 ri_o, vec3 ri_d, float t) {
    return ri_o.z + t * ri_d.z - cos(sqrt(pow(ri_o.x + t * ri_d.x, 2) + pow(ri_o.y + t * ri_d.y, 2)));
}

float objective_derivative(vec3 ri_o, vec3 ri_d, float t) {
    float num = 2 * ri_d.x * (t * ri_d.x + ri_o.x) + 2 * ri_d.y * (t * ri_d.y + ri_o.y);
    num *= sin(sqrt(pow(t * ri_d.x + ri_o.x, 2) + pow(t * ri_d.y + ri_o.y, 2)));
    float den = 2 * sqrt(pow(t * ri_d.x + ri_o.x, 2) + pow(t * ri_d.y + ri_o.y, 2));
    return num / den + ri_d.z;
}

float intersect_water(vec3 ri_o, vec3 ri_d) {
    const float epsilon = 1e-4;
    const int max_iter = 100;
    float error = 10.0;
    float t = 0;
    int iter = 0;
    
    // Newton's method
    while (iter < max_iter && error > epsilon) {
        t = t - objective_function(ri_o, ri_d, t) / objective_derivative(ri_o, ri_d, t);
        error = abs(objective_function(ri_o, ri_d, t));
        iter++;
    }

    return t;
}

vec3 intersect_geometry(vec3 rt_o, vec3 rt_d, sampler2D shadowMap) {
    rt_o = (view * vec4(rt_o, 1)).xyz;
    rt_d = (view * vec4(rt_d, 1)).xyz;

    const float epsilon = 1e-4;
    const int max_iter = 100;
    float error = 10.0;
    float t = 0;
    int iter = 0;
    float last_dist = 2, dist = 1;
    float f, last_f;

    vec3 p_estimate = rt_o + last_dist * rt_d;
    vec2 projected_p = ((projection * vec4(p_estimate, 1) + 1.0) * 0.5).xy;
    vec3 p_proj_pe = p_estimate - texture(shadowMap, projected_p).xyz;
    last_f = length(p_proj_pe);  

    while (iter < max_iter && error > epsilon) {
        p_estimate = rt_o + dist * rt_d;
        projected_p = ((projection * vec4(p_estimate, 1) + 1.0) * 0.5).xy;
        p_proj_pe = p_estimate - texture(shadowMap, projected_p).xyz;
        f = length(p_proj_pe);
        float next_dist = dist + f * (dist - last_dist) / (f - last_f);

        last_dist = dist;
        last_f = f;
        dist = next_dist;
        error = abs(f);
        iter++;
    }

    return p_estimate;
}

void main() {
    gl_Position = vec4(geomPos[0], 1);
    EmitVertex();
    return;

    vec3 ri_o = lightPos;
    // TODO: use gl_Position instead of geomPos
    vec3 ri_d = normalize(geomPos[0] - lightPos);

    // Find time where ray hits the water's surface
    float intersect = intersect_water(ri_o, ri_d);

    // Get the refracted ray
    vec3 rt_o = ri_o + ri_d * intersect;
    vec3 surface_normal = normalize(surface_gradient(rt_o));
    // Indices of refraction for air and salt water
    const float n_air = 1.0, n_water = 1.34;
    // Snell's law
    // float theta_i = arccos(dot(-ri_d, surface_normal));
    // float theta_r = arcsin(n_air / n_water * sin(theta_i));
    vec3 rt_d = normalize(refract(ri_d, surface_normal, n_air / n_water));

    // Intersect the refracted ray with the scene geometry
    vec3 geometry_intersect = intersect_geometry(rt_o, rt_d, shadowMap);

    gl_Position = vec4(geometry_intersect, 1);
    // TODO: scale point size based on distance
    gl_PointSize = 10;
    EmitVertex();
}
