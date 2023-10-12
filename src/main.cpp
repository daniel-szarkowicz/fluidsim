#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

void GLAPIENTRY message_callback(GLenum source, GLenum type, GLuint id,
                                GLenum severity, GLsizei length,
                                const GLchar* message, const void* userParam) {
    fprintf(stderr,
            "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type,
            severity, message);
}

int main(void) {
    if (!glfwInit()) {
        fprintf(stderr, "GLFW init error\n");
        return 1;
    }

    auto window = glfwCreateWindow(640, 480, "fluidsim", NULL, NULL);
    if (!window) {
        fprintf(stderr, "GLFW window error\n");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);

    glewInit();
    auto renderer = glGetString(GL_RENDERER);
    auto version = glGetString(GL_VERSION);
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version: %s\n", version);

    //glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(message_callback, 0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    auto vertex_shader = R"(
        #version 400
        in vec3 pos;
        in vec3 color;

        out vec3 vColor;

        void main() {
            gl_Position = vec4(pos, 1.0);
            vColor = color;
        }
    )";

    auto geometry_shader = R"(
        #version 400
        layout(points) in;
        layout(triangle_strip, max_vertices = 4) out;

        uniform float radius;

        in vec3 vColor[];
        out vec3 fColor;
        out vec3 fCenter;
        out vec3 fPosition;

        void main() {
            fColor = vColor[0];
            fCenter = gl_in[0].gl_Position.xyz;

            gl_Position = gl_in[0].gl_Position + vec4(-radius, -radius, 0.0, 0.0);
            fPosition = gl_Position.xyz;
            EmitVertex();

            gl_Position = gl_in[0].gl_Position + vec4(-radius, radius, 0.0, 0.0);
            fPosition = gl_Position.xyz;
            EmitVertex();

            gl_Position = gl_in[0].gl_Position + vec4(radius, -radius, 0.0, 0.0);
            fPosition = gl_Position.xyz;
            EmitVertex();

            gl_Position = gl_in[0].gl_Position + vec4(radius, radius, 0.0, 0.0);
            fPosition = gl_Position.xyz;
            EmitVertex();
            EndPrimitive();
        }
    )";

    auto fragment_shader = R"(
        #version 400
        uniform float radius;

        in vec3 fColor;
        in vec3 fCenter;
        in vec3 fPosition;
        out vec4 frag_color;

        void main() {
            if (distance(fCenter, fPosition) < radius) {
                frag_color = vec4(fColor, 1.0);
            } else {
                frag_color = vec4(0.0, 0.0, 0.0, 0.0);
            }
        }
    )";

    auto vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_shader, NULL);
    glCompileShader(vs);

    auto gs = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(gs, 1, &geometry_shader, NULL);
    glCompileShader(gs);

    auto fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment_shader, NULL);
    glCompileShader(fs);

    auto shader_program = glCreateProgram();
    glAttachShader(shader_program, vs);
    glAttachShader(shader_program, gs);
    glAttachShader(shader_program, fs);
    glLinkProgram(shader_program);

    float points[] = {
        0.0,  0.5,  0.0, 1.0, 0.0, 0.0, // red
        0.5,  -0.5, 0.0, 0.0, 1.0, 0.0, // green
        -0.5, -0.5, 0.0, 0.0, 0.0, 1.0  // blue
    };

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    auto posAttrib = glGetAttribLocation(shader_program, "pos");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          NULL);
    auto colorAttrib = glGetAttribLocation(shader_program, "color");
    glEnableVertexAttribArray(colorAttrib);
    glVertexAttribPointer(colorAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void*)(3 * sizeof(float)));

    auto radiusUniform = glGetUniformLocation(shader_program, "radius");
    glProgramUniform1f(shader_program, radiusUniform, 0.1);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shader_program);
        glBindVertexArray(vao);
        glDrawArrays(GL_POINTS, 0, 3);
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
