#include "ssbo.hpp"

SSBO::SSBO(GLuint binding, GLenum usage) : binding(binding), usage(usage) {
    glGenBuffers(1, &buffer_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer_id);
}

bool SSBO::resize(GLsizeiptr new_size) {
    GLint size;
    glGetNamedBufferParameteriv(buffer_id, GL_BUFFER_SIZE, &size);
    if (size < new_size) {
        glNamedBufferData(buffer_id, new_size, NULL, usage);
        return true;
    }
    return false;
}

void SSBO::set_data(GLsizeiptr data_size, void* data) {
    glNamedBufferSubData(buffer_id, 0, data_size, data);
}

void SSBO::bind() {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, buffer_id);
}
