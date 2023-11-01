#include "shader.hpp"
#include <fstream>
#include <glm/detail/qualifier.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>
#include <stdexcept>

Shader::builder::builder()
    : vsrc(nullptr), vfile(nullptr), gsrc(nullptr), gfile(nullptr),
      fsrc(nullptr), ffile(nullptr), csrc(nullptr), cfile(nullptr),
      graphics(false), compute(false) {}

#define builder_function(fn_name, name, src, file, goodtype, badtype,          \
                         variable)                                             \
    Shader::builder& Shader::builder::fn_name(const char* input) {             \
        if (src) {                                                             \
            throw std::invalid_argument(name " shader source already set!");   \
        }                                                                      \
        if (file) {                                                            \
            throw std::invalid_argument(name                                   \
                                        " shader file name already set!");     \
        }                                                                      \
        if (badtype) {                                                         \
            throw std::invalid_argument(                                       \
                "Cannot combine graphics and compute shaders!");               \
        }                                                                      \
        goodtype = true;                                                       \
        variable = input;                                                      \
        return *this;                                                          \
    }


builder_function(vertex_source, "Vertex", vsrc, vfile, graphics, compute, vsrc);
builder_function(vertex_file, "Vertex", vsrc, vfile, graphics, compute, vfile);
builder_function(geometry_source, "Geometry", gsrc, gfile, graphics, compute,
                 gsrc);
builder_function(geometry_file, "Geometry", gsrc, gfile, graphics, compute,
                 gfile);
builder_function(fragment_source, "Fragment", fsrc, ffile, graphics, compute,
                 fsrc);
builder_function(fragment_file, "Fragment", fsrc, ffile, graphics, compute,
                 ffile);
builder_function(compute_source, "Compute", csrc, cfile, compute, graphics,
                 csrc);
builder_function(compute_file, "Compute", csrc, cfile, compute, graphics,
                 cfile);

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

Shader Shader::builder::build() {
    if (graphics) {
        if ((!vsrc && !vfile) || (!fsrc && !ffile)) {
            throw std::invalid_argument(
                "Graphics shaders require vertex and fragment shaders");
        }
        load_shader(vertex_id, vsrc, vfile, GL_VERTEX_SHADER);
        load_shader(fragment_id, fsrc, ffile, GL_FRAGMENT_SHADER);
        GLuint geometry_id;
        if (gsrc || gfile) {
            load_shader(gid, gsrc, gfile, GL_GEOMETRY_SHADER);
            geometry_id = gid;
        }
        GLuint program_id = glCreateProgram();
        glAttachShader(program_id, vertex_id);
        if (gsrc || gfile) {
            glAttachShader(program_id, geometry_id);
        }
        glAttachShader(program_id, fragment_id);
        glLinkProgram(program_id);
        return Shader(program_id);
    } else if (compute) {
        load_shader(compute_id, csrc, cfile, GL_COMPUTE_SHADER);
        GLuint program_id = glCreateProgram();
        glAttachShader(program_id, compute_id);
        glLinkProgram(program_id);
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
