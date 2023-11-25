#pragma once

#include <GL/glew.h>
#include <sstream>

class Shader {
protected:
    Shader(GLuint program_id);
public:
    GLuint program_id;

    void use();
    template <typename... Args> void uniform(const char* name, Args...);
};

class GraphicsShader: public Shader {
public:
    GraphicsShader(GLuint program_id);

    class builder {
        std::ostringstream vertex_src;
        std::ostringstream geometry_src;
        std::ostringstream fragment_src;

    public:
        builder();
        builder& vertex_source(const char* source);
        builder& vertex_file(const char* file);
        builder& geometry_source(const char* source);
        builder& geometry_file(const char* file);
        builder& fragment_source(const char* source);
        builder& fragment_file(const char* file);
        GraphicsShader build();
    };
};

class ComputeShader: public Shader {
public:
    ComputeShader(GLuint program_id);

    class builder {
    public:
        builder();
        std::ostringstream compute_src;
        builder& compute_source(const char* source);
        builder& compute_file(const char* file);
        ComputeShader build();
    };

    void dispatch_workgroups(GLuint x = 1, GLuint y = 1, GLuint z = 1);
    void dispatch_executions(GLuint x = 1, GLuint y = 1, GLuint z = 1);
};
