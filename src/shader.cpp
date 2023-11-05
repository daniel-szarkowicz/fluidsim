#include "shader.hpp"
#include <fstream>
#include <glm/detail/qualifier.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>
#include <stdexcept>

Shader::builder::builder()
    : vertex_src(), geometry_src(), fragment_src(), compute_src(),
      graphics(false), compute(false) {}

#define builder_source(fn_name, src, goodtype, badtype)                        \
    Shader::builder& Shader::builder::fn_name(const char* source) {            \
        if (badtype) {                                                         \
            throw std::invalid_argument(                                       \
                "Cannot combine graphics and compute shaders!");               \
        }                                                                      \
        goodtype = true;                                                       \
        src << source << "\n";                                                 \
        return *this;                                                          \
    }

#define builder_file(fn_name, src, goodtype, badtype)                          \
    Shader::builder& Shader::builder::fn_name(const char* filename) {          \
        if (badtype) {                                                         \
            throw std::invalid_argument(                                       \
                "Cannot combine graphics and compute shaders!");               \
        }                                                                      \
        std::ifstream file(filename);                                          \
        file.exceptions(std::ifstream::failbit);                               \
        if (file.fail()) {                                                     \
            throw std::invalid_argument("Failed to open file!");               \
        }                                                                      \
        goodtype = true;                                                       \
        src << file.rdbuf() << "\n";                                           \
        return *this;                                                          \
    }

builder_source(vertex_source, vertex_src, graphics, compute);
builder_source(geometry_source, geometry_src, graphics, compute);
builder_source(fragment_source, fragment_src, graphics, compute);
builder_source(compute_source, compute_src, compute, graphics);

builder_file(vertex_file, vertex_src, graphics, compute);
builder_file(geometry_file, geometry_src, graphics, compute);
builder_file(fragment_file, fragment_src, graphics, compute);
builder_file(compute_file, compute_src, compute, graphics);

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
        /*GLint success;                                                       \
        glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);                 \
        printf("shader success: %d\n", success); */                            \
        glAttachShader(program_id, shader_id);                                 \
    } while (false);

Shader Shader::builder::build() {
    if (graphics) {
        std::string vertex_source = vertex_src.str();
        std::string geometry_source = geometry_src.str();
        std::string fragment_source = fragment_src.str();
        if (vertex_source.length() == 0 || fragment_source.length() == 0) {
            throw std::invalid_argument(
                "Graphics shaders require vertex and fragment shaders");
        }
        GLuint program_id = glCreateProgram();
        attach_shader(program_id, vertex_source, GL_VERTEX_SHADER);
        attach_shader(program_id, fragment_source, GL_FRAGMENT_SHADER);
        if (geometry_source.length() > 0) {
            attach_shader(program_id, geometry_source, GL_GEOMETRY_SHADER);
        }
        glLinkProgram(program_id);
        // GLint status;
        // glGetProgramiv(program_id, GL_LINK_STATUS, &status);
        // printf("link status: %d\n", status);
        return Shader(program_id);
    } else if (compute) {
        std::string compute_source = compute_src.str();
        GLuint program_id = glCreateProgram();
        attach_shader(program_id, compute_source, GL_COMPUTE_SHADER);
        glLinkProgram(program_id);
        // GLint status;
        // glGetProgramiv(program_id, GL_LINK_STATUS, &status);
        // printf("link status: %d\n", status);
        return Shader(program_id);
    } else {
        throw std::invalid_argument("No shader sources provided!");
    }
}

Shader::Shader(GLuint program_id): program_id(program_id) {}
void Shader::use() { glUseProgram(program_id); }

#define su_start(...)                                                          \
    template <> void Shader::uniform(const char* name, __VA_ARGS__) {          \
        GLint loc = glGetUniformLocation(program_id, name);                    \
        if (loc == -1) {                                                       \
            throw std::invalid_argument(                                       \
                std::string("Invalid uniform name: `") + name + "` !");        \
        }
#define su_end(uniform, ...)                                                   \
    uniform(loc, __VA_ARGS__);                                                 \
    }

su_start(GLfloat v0);
su_end(glUniform1f, v0);
su_start(GLfloat v0, GLfloat v1);
su_end(glUniform2f, v0, v1);
su_start(GLfloat v0, GLfloat v1, GLfloat v2);
su_end(glUniform3f, v0, v1, v2);
su_start(GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
su_end(glUniform4f, v0, v1, v2, v3);
su_start(GLint v0);
su_end(glUniform1i, v0);
su_start(GLint v0, GLint v1);
su_end(glUniform2i, v0, v1);
su_start(GLint v0, GLint v1, GLint v2);
su_end(glUniform3i, v0, v1, v2);
su_start(GLint v0, GLint v1, GLint v2, GLint v3);
su_end(glUniform4i, v0, v1, v2, v3);
su_start(GLuint v0);
su_end(glUniform1ui, v0);
su_start(GLuint v0, GLuint v1);
su_end(glUniform2ui, v0, v1);
su_start(GLuint v0, GLuint v1, GLuint v2);
su_end(glUniform3ui, v0, v1, v2);
su_start(GLuint v0, GLuint v1, GLuint v2, GLuint v3);
su_end(glUniform4ui, v0, v1, v2, v3);
// su_start(glm::vec1 v);
// su_end(glUniform1fv, 1, glm::value_ptr(v));
su_start(glm::vec2 v);
su_end(glUniform2fv, 1, glm::value_ptr(v));
su_start(glm::vec3 v);
su_end(glUniform3fv, 1, glm::value_ptr(v));
su_start(glm::vec4 v);
su_end(glUniform4fv, 1, glm::value_ptr(v));
// su_start(glm::vec<1, GLint> v);
// su_end(glUniform1iv, 1, glm::value_ptr(v));
su_start(glm::vec<2, GLint> v);
su_end(glUniform2iv, 1, glm::value_ptr(v));
su_start(glm::vec<3, GLint> v);
su_end(glUniform3iv, 1, glm::value_ptr(v));
su_start(glm::vec<4, GLint> v);
su_end(glUniform4iv, 1, glm::value_ptr(v));
// su_start(glm::vec<1, GLuint> v);
// su_end(glUniform1uiv, 1, glm::value_ptr(v));
su_start(glm::vec<2, GLuint> v);
su_end(glUniform2uiv, 1, glm::value_ptr(v));
su_start(glm::vec<3, GLuint> v);
su_end(glUniform3uiv, 1, glm::value_ptr(v));
su_start(glm::vec<4, GLuint> v);
su_end(glUniform4uiv, 1, glm::value_ptr(v));
su_start(glm::mat2x2 v);
su_end(glUniformMatrix2fv, 1, GL_FALSE, glm::value_ptr(v));
su_start(glm::mat3x3 v);
su_end(glUniformMatrix3fv, 1, GL_FALSE, glm::value_ptr(v));
su_start(glm::mat4x4 v);
su_end(glUniformMatrix4fv, 1, GL_FALSE, glm::value_ptr(v));
su_start(glm::mat2x3 v);
su_end(glUniformMatrix2x3fv, 1, GL_FALSE, glm::value_ptr(v));
su_start(glm::mat3x2 v);
su_end(glUniformMatrix3x2fv, 1, GL_FALSE, glm::value_ptr(v));
su_start(glm::mat2x4 v);
su_end(glUniformMatrix2x4fv, 1, GL_FALSE, glm::value_ptr(v));
su_start(glm::mat4x2 v);
su_end(glUniformMatrix4x2fv, 1, GL_FALSE, glm::value_ptr(v));
su_start(glm::mat3x4 v);
su_end(glUniformMatrix3x4fv, 1, GL_FALSE, glm::value_ptr(v));
su_start(glm::mat4x3 v);
su_end(glUniformMatrix4x3fv, 1, GL_FALSE, glm::value_ptr(v));
