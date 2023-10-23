#pragma once

#include <GL/glew.h>
#include <sstream>

class Shader {
public:
    GLuint program_id;

    Shader(GLuint program_id);

public:
    class builder {
        std::ostringstream vertex_src;
        std::ostringstream geometry_src;
        std::ostringstream fragment_src;
        std::ostringstream compute_src;
        bool graphics;
        bool compute;

    public:
        builder();
        builder& vertex_source(const char* source);
        builder& vertex_file(const char* file);
        builder& geometry_source(const char* source);
        builder& geometry_file(const char* file);
        builder& fragment_source(const char* source);
        builder& fragment_file(const char* file);
        builder& compute_source(const char* source);
        builder& compute_file(const char* file);
        Shader build();
    };

    void use();
    template <typename... Args> void uniform(const char* name, Args...);
};
