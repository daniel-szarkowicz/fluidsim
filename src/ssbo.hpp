#pragma once

#include <GL/glew.h>
#include <memory>

class SSBO {
public:
    GLuint binding;
    GLenum usage;
    GLuint buffer_id;

    SSBO(GLuint binding, GLenum usage = GL_DYNAMIC_COPY);
    bool resize(GLsizeiptr new_size);
    void set_data(GLsizeiptr data_size, void* data);
    void bind();

    SSBO(const SSBO&) = delete;
};

class SSBOPair {
public:
    std::shared_ptr<SSBO> input;
    std::shared_ptr<SSBO> output;

    SSBOPair(GLuint input_binding, GLuint output_binding,
             GLenum usage = GL_DYNAMIC_COPY);

    void bind_and_swap();

    SSBOPair(const SSBOPair&) = delete;
};
