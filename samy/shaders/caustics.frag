#version 330 core

struct CausticVertex {
    vec3 position, normal, refracted_ray;
};

layout (std140) uniform Caustics {
    CausticVertex vertices[1024];
};

uniform sampler2D world_positions;
uniform vec3 eye;

in vec4 fragPos;
in vec4 fragWorldPos;
flat in uvec3 fragIndices;

out vec4 color;

void main() {
    // color = vec4(1,1,1,1); return;
    // Check indices sent correctly
    // color = vec4(float(index[0]) + 0.5, float(index[1]) / 32.0, float(index[2]) / 2.0, 1); return;
    if (fragWorldPos.y > 5) discard;
    
    // Get world position of fragment
    vec3 worldPos = texture(world_positions, (fragPos.xy / fragPos.w + 1) * 0.5).xyz;
    // Check world position is correct
    // color = vec4(worldPos, 1); return;

    // Transform point into local coordinate system
    vec3 v[3] = vec3[3](vertices[fragIndices.x].position, vertices[fragIndices.y].position, vertices[fragIndices.z].position);
    vec3 n[3] = vec3[3](vertices[fragIndices.x].normal, vertices[fragIndices.y].normal, vertices[fragIndices.z].normal);
    vec3 r[3] = vec3[3](vertices[fragIndices.x].refracted_ray, vertices[fragIndices.y].refracted_ray, vertices[fragIndices.z].refracted_ray);

    // Check values are correct
    // color = vec4(
    //     dot(v[0], vec3(1)) + 4.5006111,
    //     dot(v[1], vec3(1)) + 3.172778,
    //     dot(v[2], vec3(1)) + 4.383235,
    //     1
    // );
    // return;

    vec3 basis_x = normalize(v[1] - v[0]);
    vec3 basis_y = normalize(cross(basis_x, v[2] - v[0]));
    // vec3 basis_y = normalize(n[0] + n[1] + n[2]);
    vec3 basis_z = cross(basis_x, basis_y);
    
    // Check basis vectors
    // color = vec4(basis_x - vec3(-.5, -.5, .5), 1); return;
    // color = vec4(basis_y - vec3(0.172167, 0.2404, -0.5), 1); return;
    // color = vec4(basis_z - vec3(-1.2404, 0.172167, -0.5), 1); return;

    // Change of basis matrix (inverse of orthonormal mat3 is transpose)
    mat3 cob = transpose(mat3(basis_x, basis_y, basis_z));

    // color = vec4(cob[0] - vec3(-.5, 0.172167, -1.2404), 1); return;
    // color = vec4(cob[1] - vec3(-.5,  0.2404, 0.172167), 1); return;
    // color = vec4(cob[2] - vec3(.5, -0.5, -0.5), 1); return;

    vec3 p = cob * worldPos;
    v[0] = cob * v[0];
    v[1] = cob * v[1];
    v[2] = cob * v[2];
    r[0] = cob * r[0];
    r[1] = cob * r[1];
    r[2] = cob * r[2];

    // color = vec4(v[0], 1) + vec4(10.5, -4.620207, -17.654556, 0); return;    
    // color = vec4(v[1], 1) + vec4(9.16667, -4.620207, -17.654556, 0); return;    
    // color = vec4(v[2], 1) + vec4(10.5, -4.620207, -15.853729, 0); return;    

    float offset = v[0].y;
    p.y -= offset;
    v[0].y -= offset;
    v[1].y -= offset;
    v[2].y -= offset;

    // color = vec4(v[0].y, v[1].y, v[2].y, 1) + vec4(0.5, 0.5, 0.5, 0); return;

    // Scale r_i so y-components are one
    r[0] /= r[0].y;
    r[1] /= r[1].y;
    r[2] /= r[2].y;

    // Compute barycentric coordinates
    // do optimizations in 4.1
    float alpha = p.y;
    vec3 c[3];
    for (int i = 0; i < 3; i++) {
        c[i] = v[i] + alpha * r[i];
    }

    vec3 beta;
    beta.x = p.x * (c[2].z - c[1].z) + p.z * (c[1].x - c[2].x) + c[1].z * c[2].x - c[1].x * c[2].z;
    beta.y = p.x * (c[0].z - c[2].z) + p.z * (c[2].x - c[0].x) + c[2].z * c[0].x - c[2].x * c[0].z;
    beta.z = p.x * (c[1].z - c[0].z) + p.z * (c[0].x - c[1].x) + c[0].z * c[1].x - c[0].x * c[1].z;

    // Test if point is in caustic volume
    // if (any(greaterThanEqual(beta, vec3(0)))) {
    if (!(all(greaterThanEqual(beta, vec3(0))) || all(lessThanEqual(beta, vec3(0))))) {
        discard;
    }

    // Intensity calculation
    float area_specular = length(cross(v[1] - v[0], v[2] - v[0]));
    float area_caustic = length(cross(c[1] - c[0], c[2] - c[0]));
    float intensity = min(area_specular / area_caustic, 1.0);

    color = vec4(vec3(1), intensity / 2);
}
