#pragma once

#include <GL/glew.h>
#include <sstream>
#include <unordered_set>
#include "ssbo.hpp"

class Shader {
protected:
    Shader(GLuint program_id);
public:
    GLuint program_id;

    void use();
    template <typename... Args> void uniform(const char* name, Args...);

    Shader(const Shader&) = delete;
};

class GraphicsShader: public Shader {
    static GLuint empty_vao();
    std::unordered_set<std::shared_ptr<SSBO>> ssbos;

public:
    GraphicsShader(
        GLuint program_id,
        std::unordered_set<std::shared_ptr<SSBO>> ssbos
    );

    class builder {
        std::ostringstream vertex_src;
        std::ostringstream geometry_src;
        std::ostringstream fragment_src;
        std::unordered_set<std::shared_ptr<SSBO>> ssbos;

    public:
        builder();
        builder& vertex_source(const char* source);
        builder& vertex_file(const char* file);
        builder& geometry_source(const char* source);
        builder& geometry_file(const char* file);
        builder& fragment_source(const char* source);
        builder& fragment_file(const char* file);
        builder& ssbo(std::shared_ptr<SSBO> ssbo);
        GraphicsShader build();
    };

    void draw_instanced(GLenum type, GLsizei vertex_count,
                        GLsizei instance_count);
};

class ComputeShader: public Shader {
    std::unordered_set<std::shared_ptr<SSBO>> ssbos;
    std::unordered_set<std::shared_ptr<SSBOPair>> ssbopairs;

public:
    ComputeShader(
        GLuint program_id,
        std::unordered_set<std::shared_ptr<SSBO>> ssbos,
        std::unordered_set<std::shared_ptr<SSBOPair>> ssbopairs
    );

    class builder {
        std::ostringstream compute_src;
        std::unordered_set<std::shared_ptr<SSBO>> ssbos;
        std::unordered_set<std::shared_ptr<SSBOPair>> ssbopairs;

    public:
        builder();
        builder& compute_source(const char* source);
        builder& compute_file(const char* file);
        builder& ssbo(std::shared_ptr<SSBO> ssbo);
        builder& ssbopair(std::shared_ptr<SSBOPair> ssbopair);
        ComputeShader build();
    };

    void dispatch_workgroups(GLuint x = 1, GLuint y = 1, GLuint z = 1);
    void dispatch_executions(GLuint x = 1, GLuint y = 1, GLuint z = 1);
};
