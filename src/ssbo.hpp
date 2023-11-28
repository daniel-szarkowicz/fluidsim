#include <GL/glew.h>

class SSBO {
public:
    GLuint binding;
    GLenum usage;
    GLuint buffer_id;

    SSBO(GLuint binding, GLenum usage = GL_DYNAMIC_COPY);
    bool resize(GLsizeiptr new_size);
    void set_data(GLsizeiptr data_size, void* data);
    void bind();
};
