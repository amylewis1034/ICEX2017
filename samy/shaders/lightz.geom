#version 330 core

layout (points) in;
layout (points, max_vertices = 4) out;

in vec3 geomPos[];

uniform mat4 projection;
uniform mat4 view;
uniform mat4 lp; // light space projection matrix
uniform mat4 lv;  // light space matrix
uniform vec3 lightPos;
uniform float time;
uniform sampler2D shadowMap;

const float near = 0.1, far = 20.0;

float linearize_depth(float depth, float near, float far) {
    float z = depth * 2.0 - 1.0;
    return 2.0 * near * far / (far + near - z * (far - near));
}

// float surface(float x, float y) {
//     return cos(sqrt(x * x + y * y));
// }

// vec3 surface_gradient(vec3 point) {
//     float x = point.x, y = point.y, z = point.z;
//     float groot = sqrt(x*x + y*y);
//     float singroot = sin(groot);

//     return vec3(x*singroot / groot, y * singroot / groot, 1);
// }

// float objective_function(vec3 ri_o, vec3 ri_d, float t) {
//     return ri_o.z + t * ri_d.z - cos(sqrt(pow(ri_o.x + t * ri_d.x, 2) + pow(ri_o.y + t * ri_d.y, 2)));
// }

// float objective_derivative(vec3 ri_o, vec3 ri_d, float t) {
//     float num = 2 * ri_d.x * (t * ri_d.x + ri_o.x) + 2 * ri_d.y * (t * ri_d.y + ri_o.y);
//     num *= sin(sqrt(pow(t * ri_d.x + ri_o.x, 2) + pow(t * ri_d.y + ri_o.y, 2)));
//     float den = 2 * sqrt(pow(t * ri_d.x + ri_o.x, 2) + pow(t * ri_d.y + ri_o.y, 2));
//     return num / den + ri_d.z;
// }

float surface(float x, float y) {
    return -10.0;
}

vec3 surface_gradient(vec3 point) {
    return vec3(0, 0, 1);
}

float objective_function(vec3 ri_o, vec3 ri_d, float t) {
    return -10 - (ri_o.z + t * ri_d.z);
}

float objective_derivative(vec3 ri_o, vec3 ri_d, float t) {
    return -ri_d.z;
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
    // rt_o = (lv * vec4(rt_o, 1)).xyz;
    // rt_d = (lv * vec4(rt_d, 0)).xyz;

    // gl_Position = projection * view * vec4(rt_o + 5 * rt_d, 1);
    // EmitVertex();
    // EndPrimitive();

    const float epsilon = 1e-4;
    const int max_iter = 1000;
    float error = 10.0;
    float t = 0;
    int iter = 0;
    float last_dist = 10, dist = 5.5;
    float f, last_f;

    vec3 p_estimate = rt_o + last_dist * rt_d;
    vec4 projected_p = lp * lv * vec4(p_estimate, 1);
    vec2 shifted_p = (projected_p.xy + 1.0) * 0.5;

    float depth = linearize_depth(texture(shadowMap, shifted_p).x, near, far);
    return rt_o + (texture(shadowMap, shifted_p).x * far - 10) * rt_d;

    // vec2 projected_p = ((lp * vec4(p_estimate, 1) + 1.0) * 0.5).xy;
    last_f = linearize_depth((projected_p.z + 1) * .5, near, far) - linearize_depth(texture(shadowMap, shifted_p).x, near, far);
    error = abs(last_f);

    while (iter < max_iter && error > epsilon) {
        p_estimate = rt_o + dist * rt_d;
        projected_p = lp * lv * vec4(p_estimate, 1);
        shifted_p = (projected_p.xy + 1.0) * 0.5;
        f = linearize_depth((projected_p.z + 1) * .5, near, far) - linearize_depth(texture(shadowMap, shifted_p).x, near, far);
        float next_dist = dist - f * (dist - last_dist) / (f - last_f);

        last_dist = dist;
        last_f = f;
        dist = next_dist;
        error = abs(f);
        iter++;
    }

    return p_estimate;
}

float f(vec3 rt_o, vec3 rt_d, sampler2D shadowMap, float t) {
    vec3 p_estimate = rt_o + t * rt_d;
    vec4 projected_p = lp * lv * vec4(p_estimate, 1);
    vec2 shifted_p = (projected_p.xy + 1.0) * 0.5;
    return (projected_p.z + 1) * 0.5 * far - texture(shadowMap, shifted_p).x * far;
    // return (projected_p.z + 1) * 0.5 - texture(shadowMap, shifted_p).r;
}

vec3 intersect_geometry2(vec3 rt_o, vec3 rt_d, sampler2D shadowMap) {
    const float epsilon = 1e-4;
    const float tolerance = 1e-4;
    const int max_iter = 1000;
    int iter = 0;

    float a = 0, b = 30;
    float c;

    float fa = f(rt_o, rt_d, shadowMap, a);
    float fb = f(rt_o, rt_d, shadowMap, b);

    if (sign(fa) == sign(fb)) {
        return rt_o;
    }

    while (iter < max_iter) {
        c = (a + b) / 2;

        float fc = f(rt_o, rt_d, shadowMap, c);

        // if (fc < epsilon || (b - a) / 2 < tolerance) {
        //     break;
        // }
        if ((b - a) / 2 < tolerance) {
            break;
        }

        if (sign(fc) == sign(fa)) {
            a = c;
            fa = fc;
        }
        else {
            b = c;
            fb = fc;
        }

        // gl_Position = projection * view * vec4(rt_o + rt_d * c, 1);
        // EmitVertex();
        // EndPrimitive();

        iter++;
    }

    return rt_o + rt_d * c;
}

void main() {
    // gl_Position = projection * view * inverse(lv) * gl_in[0].gl_Position;
    // EmitVertex();
    // EndPrimitive();
    // return;

    vec3 photon = (inverse(lv) * gl_in[0].gl_Position).xyz;
    // gl_Position = projection * view * vec4(photon, 1);
    // EmitVertex();
    // EndPrimitive();

    vec3 ri_o = lightPos;
    vec3 ri_d = normalize(photon - lightPos);
    // vec3 ri_o = photon;
    // vec3 ri_d = vec3(0, 0, -1);

    // Find time where ray hits the water's surface
    float intersect = intersect_water(ri_o, ri_d);

    // Get the refracted ray
    vec3 rt_o = ri_o + ri_d * intersect;

    // gl_Position = projection * view * vec4(rt_o, 1);
    // EmitVertex();
    // EndPrimitive();
    // return;

    vec3 surface_normal = normalize(surface_gradient(rt_o));
    // Indices of refraction for air and salt water
    const float n_air = 1.0, n_water = 1.0;
    // const float n_air = 1.0, n_water = 1.34;
    // Snell's law
    // float theta_i = arccos(dot(-ri_d, surface_normal));
    // float theta_r = arcsin(n_air / n_water * sin(theta_i));
    vec3 rt_d = normalize(refract(ri_d, surface_normal, n_air / n_water));

    // gl_Position = projection * view * vec4(rt_o + 5 * rt_d, 1);
    // EmitVertex();
    // EndPrimitive();

    // Intersect the refracted ray with the scene geometry
    vec3 geometry_intersect = intersect_geometry2(rt_o, rt_d, shadowMap);

    gl_Position = projection * view * vec4(geometry_intersect, 1);
    // TODO: scale point size based on distance
    // gl_PointSize = 10;
    EmitVertex();
    EndPrimitive();
}
