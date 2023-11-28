#include <GL/glew.h>

class SSBO {
public:
    GLuint binding;
    GLenum usage;
    GLuint buffer_id;

    SSBO(GLuint binding, GLenum usage = GL_DYNAMIC_COPY);
    void resize(GLsizeiptr size);
    void set_data(GLsizeiptr size, void* data);
    void bind();
};
