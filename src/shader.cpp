#include "shader.hpp"
#include <fstream>
#include <glm/detail/qualifier.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>

GraphicsShader::builder::builder()
    : vertex_src(), geometry_src(), fragment_src() {}
ComputeShader::builder::builder(): compute_src() {}

#define builder_source(type_name, fn_name, src)                                \
    type_name::builder& type_name::builder::fn_name(const char* source) {      \
        src << source << "\n";                                                 \
        return *this;                                                          \
    }

#define builder_file(type_name, fn_name, src)                                  \
    type_name::builder& type_name::builder::fn_name(const char* filename) {    \
        std::ifstream file(filename);                                          \
        if (file.fail()) {                                                     \
            fprintf(stderr,                                                    \
                    "[ERROR]: Failed to open shader file %s\n", filename);     \
            exit(1);                                                           \
        }                                                                      \
        src << file.rdbuf() << "\n";                                           \
        return *this;                                                          \
    }

builder_source(GraphicsShader, vertex_source, vertex_src);
builder_source(GraphicsShader, geometry_source, geometry_src);
builder_source(GraphicsShader, fragment_source, fragment_src);
builder_source(ComputeShader, compute_source, compute_src);

builder_file(GraphicsShader, vertex_file, vertex_src);
builder_file(GraphicsShader, geometry_file, geometry_src);
builder_file(GraphicsShader, fragment_file, fragment_src);
builder_file(ComputeShader, compute_file, compute_src);

#define load_shader(shader_id, src, fname, type)                               \
    GLuint shader_id;                                                          \
    if (fname) {                                                               \
        std::ifstream file(fname);                                             \
        std::ostringstream ss;                                                 \
        ss << file.rdbuf();                                                    \
        std::string s = (ss.str());                                            \
        src = s.c_str();                                                       \
        shader_id = glCreateShader(type);                                      \
        glShaderSource(shader_id, 1, &src, NULL);                              \
        glCompileShader(shader_id);                                            \
    } else {                                                                   \
        shader_id = glCreateShader(type);                                      \
        glShaderSource(shader_id, 1, &src, NULL);                              \
        glCompileShader(shader_id);                                            \
    }

#define attach_shader(program_id, source, type)                                \
    do {                                                                       \
        const char* str = source.c_str();                                      \
        GLuint shader_id = glCreateShader(type);                               \
        glShaderSource(shader_id, 1, &str, NULL);                              \
        glCompileShader(shader_id);                                            \
        glAttachShader(program_id, shader_id);                                 \
    } while (false);

GraphicsShader GraphicsShader::builder::build() {
    std::string vertex_source = vertex_src.str();
    std::string geometry_source = geometry_src.str();
    std::string fragment_source = fragment_src.str();
    if (vertex_source.length() == 0 || fragment_source.length() == 0) {
        fprintf(
            stderr,
            "[ERROR]: Graphics shaders require vertex and fragment shaders\n"
        );
        exit(1);
    }
    GLuint program_id = glCreateProgram();
    attach_shader(program_id, vertex_source, GL_VERTEX_SHADER);
    attach_shader(program_id, fragment_source, GL_FRAGMENT_SHADER);
    if (geometry_source.length() > 0) {
        attach_shader(program_id, geometry_source, GL_GEOMETRY_SHADER);
    }
    glLinkProgram(program_id);
    return GraphicsShader(program_id);
}

ComputeShader ComputeShader::builder::build() {
    std::string compute_source = compute_src.str();
    GLuint program_id = glCreateProgram();
    attach_shader(program_id, compute_source, GL_COMPUTE_SHADER);
    glLinkProgram(program_id);
    return ComputeShader(program_id);
}

Shader::Shader(GLuint program_id): program_id(program_id) {}
GraphicsShader::GraphicsShader(GLuint program_id): Shader(program_id) {}
ComputeShader::ComputeShader(GLuint program_id)
    : Shader(program_id), ssbos(), ssbopairs() {}

void Shader::use() { glUseProgram(program_id); }

void ComputeShader::dispatch_workgroups(GLuint x, GLuint y, GLuint z) {
    use();
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    for (auto ssbo : ssbos) {
        ssbo->bind();
    }
    for (auto ssbopair : ssbopairs) {
        ssbopair->bind_and_swap();
    }
    glDispatchCompute(x, y, z);
}

void ComputeShader::dispatch_executions(GLuint x, GLuint y, GLuint z) {
    GLint w[3];
    glGetProgramiv(program_id, GL_COMPUTE_WORK_GROUP_SIZE, w);
    dispatch_workgroups(
        (x + w[0] - 1) / w[0],
        (y + w[1] - 1) / w[1],
        (z + w[2] - 1) / w[2]
    );
}

#define su_start(...)                                                          \
    template <> void Shader::uniform(const char* name, __VA_ARGS__) {          \
        GLint loc = glGetUniformLocation(program_id, name);                    \
        if (loc == -1) {                                                       \
            fprintf(stderr, "[WARNING]: Invalid uniform name: %s\n", name);    \
        }
#define su_end(uniform, ...)                                                   \
    glProgramUniform##uniform(program_id, loc, __VA_ARGS__);                   \
    }

su_start(GLfloat v0);
su_end(1f, v0);
su_start(GLfloat v0, GLfloat v1);
su_end(2f, v0, v1);
su_start(GLfloat v0, GLfloat v1, GLfloat v2);
su_end(3f, v0, v1, v2);
su_start(GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
su_end(4f, v0, v1, v2, v3);
su_start(GLint v0);
su_end(1i, v0);
su_start(GLint v0, GLint v1);
su_end(2i, v0, v1);
su_start(GLint v0, GLint v1, GLint v2);
su_end(3i, v0, v1, v2);
su_start(GLint v0, GLint v1, GLint v2, GLint v3);
su_end(4i, v0, v1, v2, v3);
su_start(GLuint v0);
su_end(1ui, v0);
su_start(GLuint v0, GLuint v1);
su_end(2ui, v0, v1);
su_start(GLuint v0, GLuint v1, GLuint v2);
su_end(3ui, v0, v1, v2);
su_start(GLuint v0, GLuint v1, GLuint v2, GLuint v3);
su_end(4ui, v0, v1, v2, v3);
// su_start(glm::vec1 v);
// su_end(1fv, 1, glm::value_ptr(v));
su_start(glm::vec2 v);
su_end(2fv, 1, glm::value_ptr(v));
su_start(glm::vec3 v);
su_end(3fv, 1, glm::value_ptr(v));
su_start(glm::vec4 v);
su_end(4fv, 1, glm::value_ptr(v));
// su_start(glm::vec<1, GLint> v);
// su_end(1iv, 1, glm::value_ptr(v));
su_start(glm::vec<2, GLint> v);
su_end(2iv, 1, glm::value_ptr(v));
su_start(glm::vec<3, GLint> v);
su_end(3iv, 1, glm::value_ptr(v));
su_start(glm::vec<4, GLint> v);
su_end(4iv, 1, glm::value_ptr(v));
// su_start(glm::vec<1, GLuint> v);
// su_end(1uiv, 1, glm::value_ptr(v));
su_start(glm::vec<2, GLuint> v);
su_end(2uiv, 1, glm::value_ptr(v));
su_start(glm::vec<3, GLuint> v);
su_end(3uiv, 1, glm::value_ptr(v));
su_start(glm::vec<4, GLuint> v);
su_end(4uiv, 1, glm::value_ptr(v));
su_start(glm::mat2x2 v);
su_end(Matrix2fv, 1, GL_FALSE, glm::value_ptr(v));
su_start(glm::mat3x3 v);
su_end(Matrix3fv, 1, GL_FALSE, glm::value_ptr(v));
su_start(glm::mat4x4 v);
su_end(Matrix4fv, 1, GL_FALSE, glm::value_ptr(v));
su_start(glm::mat2x3 v);
su_end(Matrix2x3fv, 1, GL_FALSE, glm::value_ptr(v));
su_start(glm::mat3x2 v);
su_end(Matrix3x2fv, 1, GL_FALSE, glm::value_ptr(v));
su_start(glm::mat2x4 v);
su_end(Matrix2x4fv, 1, GL_FALSE, glm::value_ptr(v));
su_start(glm::mat4x2 v);
su_end(Matrix4x2fv, 1, GL_FALSE, glm::value_ptr(v));
su_start(glm::mat3x4 v);
su_end(Matrix3x4fv, 1, GL_FALSE, glm::value_ptr(v));
su_start(glm::mat4x3 v);
su_end(Matrix4x3fv, 1, GL_FALSE, glm::value_ptr(v));
