#include "WaterMesh.hpp"
#include "Transform.hpp"
#include <GameObject.hpp>
#include <World.hpp>
#include <Graphics/GLShader.hpp>

#include <fstream>
#include <iostream>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/epsilon.hpp>
#include <limits>

extern World *world;

const float left = -30.0f, right = 30.0f;
const float bot = -30.0f, top = 30.0f;

// static float single_wave(float x, float y, float t, const Wave &wave) {
//     float theta = glm::dot(wave.direction, glm::vec2(x, y));
//     return 15.0f + wave.amplitude * glm::sin(theta * wave.frequency + t * wave.phase);
// }

// static glm::vec3 wave_normal(float x, float y, float t, const Wave &wave) {
//     float theta = glm::dot(wave.direction, glm::vec2(x, y));
//     float dx = wave.amplitude * wave.direction.x * wave.frequency
//         * glm::cos(theta * wave.frequency + t * wave.phase);
//     float dy = wave.amplitude * wave.direction.y * wave.frequency
//         * glm::cos(theta * wave.frequency + t * wave.phase);
    
//     return glm::normalize(glm::vec3(-dx, 1.0f, -dy));
// }
static float single_wave(float x, float y, float t, const Wave &wave) {
    float theta = glm::sqrt(glm::dot(glm::vec2(x, y), glm::vec2(x, y)));
    return 20.0f + wave.amplitude * glm::cos(theta * wave.frequency + t * wave.phase);
}

static glm::vec3 wave_normal(float x, float y, float t, const Wave &wave) {
    float theta = glm::sqrt(glm::dot(glm::vec2(x, y), glm::vec2(x, y)));
    float dx = wave.amplitude * wave.direction.x * wave.frequency * x
        * glm::sin(theta * wave.frequency + t * wave.phase) / theta;
    float dy = wave.amplitude * wave.direction.y * wave.frequency * y
        * glm::sin(theta * wave.frequency + t * wave.phase) / theta;
    
    return glm::normalize(glm::vec3(-dx, 1.0f, -dy));
}

WaterMesh::WaterMesh(int width, int height) :
    width(width), height(height), wave({0.5f, 1.0f, 1.0f, glm::vec2(1.0f, 0.0f)}) {}

WaterMesh::~WaterMesh() {}

void WaterMesh::init() {
    verts.resize(width * height);
    texcoords.resize(width * height);
    normals.resize(width * height, glm::vec3(0));
    refracted_rays.resize(width * height);
    vertices.resize(width * height, {glm::vec3(0), 0, glm::vec3(0), 0, glm::vec3(0), 0});
    // vertices.resize(width * height, {glm::vec3(0), glm::vec3(0), glm::vec3(0)});

    generate_vertices(0.0f, world->getMainlightPosition());
    generate_indices();

    bboxes.resize(indices.size() - 2);
    instance_matrices.resize(bboxes.size());
    generate_bboxes();

    cubemesh = new Mesh(RESOURCE_PATH "objs/cube.obj");
    assert(cubemesh);

    init_buffers();

    // Transform feedback shader program
    GLShader wave_shader {GL_VERTEX_SHADER, SHADER_PATH "wave_gen.vert"};
    wave_program = glCreateProgram();
    glAttachShader(wave_program, wave_shader.getHandle());

    const GLchar *feedbackVaryings[] = {"pos", "normal", "refracted_ray"};
    glTransformFeedbackVaryings(wave_program, 3, feedbackVaryings, GL_INTERLEAVED_ATTRIBS);
    
    glLinkProgram(wave_program);
    GLint success;
	GLchar infoLog[512];
    glGetProgramiv(wave_program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(wave_program, 512, NULL, infoLog);
		std::cout
			<< "ERROR::SHADER_PROGRAM::COMPILATION_FAILED\n"
			<< infoLog
			<< std::endl;
	}

    // make sure buffers stick around
    glm::vec2 grid_points[width * height];
    float xoffset = (right - left) / (width - 1);
    float yoffset = (top - bot) / (height - 1);
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            int index = j * width + i;
            float x = i * xoffset + left;
            float y = j * yoffset + bot;
            grid_points[index] = glm::vec2(x, y);
            // std::cout << glm::to_string(grid_points[index]) << std::endl;
        }
    }

    glGenVertexArrays(1, &grid_vao);
    glBindVertexArray(grid_vao);

    glGenBuffers(1, &grid_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, grid_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(grid_points), grid_points, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glGenBuffers(1, &feedback_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, feedback_vbo);
    glBufferData(GL_ARRAY_BUFFER, width * height * 3 * sizeof(glm::vec4), nullptr, GL_STATIC_READ);

    generate_water(0.0f);
}

void WaterMesh::generate_water(float t) {
    // GLuint query;
    // glGenQueries(1, &query);

    glEnable(GL_RASTERIZER_DISCARD);
    glBindBuffer(GL_ARRAY_BUFFER, feedback_vbo);    
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, feedback_vbo);

    glUseProgram(wave_program);
    glBindVertexArray(grid_vao);

    glUniform3fv(glGetUniformLocation(wave_program, "light_pos"), 1, glm::value_ptr(world->getMainlightPosition()));
    glUniform1f(glGetUniformLocation(wave_program, "time"), t);
    glUniform1f(glGetUniformLocation(wave_program, "wave.amplitude"), wave.amplitude);
    glUniform1f(glGetUniformLocation(wave_program, "wave.frequency"), wave.frequency);
    glUniform1f(glGetUniformLocation(wave_program, "wave.phase"), wave.phase);
    glUniform2f(glGetUniformLocation(wave_program, "wave.direction"), wave.direction.x, wave.direction.y);

    // glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, query);
    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, width * height);
    glEndTransformFeedback();
    // glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

    glBindVertexArray(0);
    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDisable(GL_RASTERIZER_DISCARD);

    // glFlush();

    // GLuint primitives;
    // glGetQueryObjectuiv(query, GL_QUERY_RESULT, &primitives);
    // std::cerr << "Wrote " << primitives << " primitives" << std::endl;
    // glDeleteQueries(1, &query);
    
    // VertexInfo vertex_info[width * height];
    // glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(vertex_info), vertex_info);

    // bool matches = true;
    // for (int i = 0; i < width * height; i++) {
    //     auto &v = vertex_info[i];
    //     const float epsilon = 1e-4;
    //     bool position_equal = glm::all(glm::epsilonEqual(v.position, verts[i], epsilon));
    //     bool normal_equal = glm::all(glm::epsilonEqual(v.normal, normals[i], epsilon));
    //     bool ray_equal = glm::all(glm::epsilonEqual(v.refracted_ray, refracted_rays[i], epsilon));

    //     matches = matches && position_equal && normal_equal && ray_equal;
    //     // if (!(position_equal && normal_equal && ray_equal)) {
    //     //     std::cerr << "no work :(" << std::endl;
    //     // }
    //     // std::cout << glm::to_string(v.position) << std::endl;
    //     // std::cout << glm::to_string(verts[i]) << std::endl;
    //     // std::cout << glm::to_string(v.normal) << std::endl;
    //     // std::cout << glm::to_string(normals[i]) << std::endl;
    //     // std::cout << glm::to_string(v.refracted_ray) << std::endl;
    //     // std::cout << glm::to_string(refracted_rays[i]) << std::endl;
    // }
    // if (!matches) {
    //     std::cerr << "Transform feedback values differ from cpu side" << std::endl;
    // }
}

void WaterMesh::update(float dt) {
    static float t = 0.0f;
    t += dt;

    generate_water(t);
    generate_vertices(t, world->getMainlightPosition());
    generate_bboxes();

    update_buffers();
}

void WaterMesh::draw() {
    glDisable(GL_CULL_FACE);
    vao.bind();
    ebo.bind();
    glDrawElements(GL_TRIANGLE_STRIP, indices.size(), GL_UNSIGNED_INT, 0);
    ebo.unbind();
    vao.unbind();
    glEnable(GL_CULL_FACE);
}

void WaterMesh::draw_caustics() {
    glBindBuffer(GL_UNIFORM_BUFFER, feedback_vbo);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, feedback_vbo);
    // glBindBufferBase(GL_UNIFORM_BUFFER, 0, this->vertex_buf[which].getHandle());
    cubemesh->draw_instanced(indices.size() - 2);
}

void WaterMesh::generate_vertices(float t, const glm::vec3 &light_position) {
    const float n_air = 1.0f, n_water = 1.33f;

    /* Create vertices and texture coordinates */
    float xoffset = (right - left) / (width - 1);
    float yoffset = (top - bot) / (height - 1);

    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            int index = j * width + i;
            float x = i * xoffset + left;
            float y = j * yoffset + bot;

            // glm::vec3 &v = vertices[index];
            // glm::vec3 &n = normals[index];
            // glm::vec3 &r = refracted_rays[index];
            // glm::vec2 &t = texcoords[index];
            glm::vec3 &v = vertices[index].position;
            glm::vec3 &n = vertices[index].normal;
            glm::vec3 &r = vertices[index].refracted_ray;

            float height = 0.0f;
            height = single_wave(x, y, t, this->wave);

            verts[index] = v = glm::vec3(x, height, y);
            normals[index] = n = wave_normal(x, y, t, this->wave);

            glm::vec3 incident = glm::normalize(v - light_position);
            refracted_rays[index] = r = glm::refract(incident, n, n_air / n_water);

			texcoords[index] = glm::vec2((float)i / width, 1.0f - (float)j / height);
            // texcoords[j * width + i] *= this->texture_scale;
        }
    }
}

void WaterMesh::generate_indices() {
    /* Create indices for rendering */
    for (int j = 0; j < height - 1; j++) {
        if (j % 2 == 0) {
            /* Even row: left to right */
            for (int i = 0; i < width; i++) {
                indices.push_back(j * width + i);
                indices.push_back((j + 1) * width + i);
            }

            if (j < height - 2) {
                indices.push_back((j + 1) * width + width - 1);
            }
        }
        else {
            /* Odd row: right to left */
            for (int i = width - 1; i >= 0; i--) {
                indices.push_back(j * width + i);
                indices.push_back((j + 1) * width + i);
            }

            if (j < height - 2) {
                indices.push_back((j + 1) * width);
            }
        }
    }
}

void WaterMesh::generate_bboxes() {
    glm::vec3 a[2], b[2], c[2];
    // a[0] = vertices[indices[0]];
    // a[1] = vertices[indices[0]] - refracted_rays[indices[0]] * (a[0].y / refracted_rays[indices[0]].y);
    // b[0] = vertices[indices[1]];
    // b[1] = vertices[indices[1]] - refracted_rays[indices[1]] * (b[0].y / refracted_rays[indices[1]].y);
    a[0] = vertices[indices[0]].position;
    a[1] = vertices[indices[0]].position - vertices[indices[0]].refracted_ray * (a[0].y / vertices[indices[0]].refracted_ray.y);
    b[0] = vertices[indices[1]].position;
    b[1] = vertices[indices[1]].position - vertices[indices[1]].refracted_ray * (b[0].y / vertices[indices[1]].refracted_ray.y);
    
    for (int i = 0; i < bboxes.size(); i++) {
        // c[0] = vertices[indices[i + 2]];
        // c[1] = vertices[indices[i + 2]] - refracted_rays[indices[i + 2]] * (c[0].y / refracted_rays[indices[i + 2]].y);
        c[0] = vertices[indices[i + 2]].position;
        c[1] = vertices[indices[i + 2]].position - vertices[indices[i + 2]].refracted_ray * (c[0].y / vertices[indices[i + 2]].refracted_ray.y);
    
        const glm::vec3 vol_verts[6] {
            a[0], a[1], b[0], b[1], c[0], c[1]
        };

        glm::vec3 min_point {std::numeric_limits<float>::max()};
        glm::vec3 max_point {std::numeric_limits<float>::min()};
        for (const auto &v : vol_verts) {
            min_point = glm::min(min_point, v);
            max_point = glm::max(max_point, v);
        }

        bboxes[i].scale = max_point - min_point;
        bboxes[i].center = (min_point + max_point) / 2.0f;
        bboxes[i].center.y -= 1.0f;

        // TODO: could access gameobject to get transforms
        glm::mat4 t, s;
        t = glm::translate(t, bboxes[i].center);
        s = glm::scale(s, bboxes[i].scale);
        instance_matrices[i] = t * s;
        // std::cout << glm::to_string(instance_matrices[i]) << std::endl;

        a[0] = b[0];
        a[1] = b[1];
        b[0] = c[0];
        b[1] = c[1];
    }
}

void WaterMesh::init_buffers() {
    /* For drawing stuff */
    positions.loadData(GL_ARRAY_BUFFER, verts.size() * sizeof(glm::vec3), verts.data(), GL_STREAM_DRAW);

    nBuf.loadData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STREAM_DRAW);

	tBuf.loadData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(glm::vec2), texcoords.data(), GL_STATIC_DRAW);

    this->instanceBuf.loadData(GL_ARRAY_BUFFER, this->instance_matrices.size() * sizeof(glm::mat3), this->instance_matrices.data(), GL_STREAM_DRAW);    

    this->instanceIndicesBuf.loadData(GL_ARRAY_BUFFER, indices.size() * sizeof(unsigned), indices.data(), GL_STATIC_DRAW);

    vao.bind();

    positions.bind();
    vao.addAttribute(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    nBuf.bind();
    vao.addAttribute(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // vertex_buf.bind();
    // vao.addAttribute(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (GLvoid *)offsetof(VertexInfo, position));
    // vao.addAttribute(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (GLvoid *)offsetof(VertexInfo, normal));

	tBuf.bind();
	vao.addAttribute(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

    vao.unbind();

    ebo.loadData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    /* Instanced caustic vao */
    glBindVertexArray(cubemesh->getVAO());
    this->instanceBuf.bind();
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (GLvoid *)0);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (GLvoid *)sizeof(glm::vec4));
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (GLvoid *)(2 * sizeof(glm::vec4)));
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (GLvoid *)(3 * sizeof(glm::vec4)));
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);

    this->instanceIndicesBuf.bind();
    glEnableVertexAttribArray(7);
    glVertexAttribIPointer(7, 3, GL_UNSIGNED_INT, sizeof(unsigned int), 0);
    glVertexAttribDivisor(7, 1);

    glBindVertexArray(0);

    /* Caustic block */
    // vertex_buf[this->which].loadData(GL_UNIFORM_BUFFER, vertices.size() * sizeof(VertexInfo), vertices.data(), GL_STREAM_DRAW);
}

void WaterMesh::update_buffers() {
    positions.loadData(GL_ARRAY_BUFFER, verts.size() * sizeof(glm::vec3), verts.data(), GL_STREAM_DRAW);
    nBuf.loadData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STREAM_DRAW);

    // this->which = 1 - this->which;
    // vertex_buf[this->which].loadData(GL_UNIFORM_BUFFER, vertices.size() * sizeof(VertexInfo), vertices.data(), GL_STREAM_DRAW);

    this->instanceBuf.loadData(GL_ARRAY_BUFFER, this->instance_matrices.size() * sizeof(glm::mat3), this->instance_matrices.data(), GL_STREAM_DRAW);    
}
