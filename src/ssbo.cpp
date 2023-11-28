#include "ssbo.hpp"

SSBO::SSBO(GLuint binding, GLenum usage) :binding(binding), usage(usage) {
    glGenBuffers(1, &buffer_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer_id);
}

void SSBO::resize(GLsizeiptr size) {
    glNamedBufferData(buffer_id, size, NULL, usage);
}

void SSBO::set_data(GLsizeiptr size, void* data) {
    glNamedBufferSubData(buffer_id, 0, size, data);
}

void SSBO::bind() {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, buffer_id);
}
