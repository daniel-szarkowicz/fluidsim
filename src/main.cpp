#include "camera.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>
#include <stdio.h>
#include <string>

using glm::vec3;

struct Sphere {
    vec3 center;
    vec3 color;
    float radius;
};

void GLAPIENTRY message_callback(GLenum source, GLenum type, GLuint id,
                                 GLenum severity, GLsizei length,
                                 const GLchar* message, const void* userParam) {
    fprintf(stderr,
            "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type,
            severity, message);
}

GLuint load_shader(const char* filename, GLenum type) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        fprintf(stderr, "ERROR: Could not open shader `%s`\n", filename);
        return 0;
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    std::string s(ss.str());
    auto source = s.c_str();

    auto shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    return shader;
}

Camera camera(60, 640.0 / 480.0, 0.1, 1000.0, vec3(0, 0, 2), vec3(0, 0, 0));
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    camera.aspect_ratio = (float)width / (float)height;
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
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glewInit();
    auto renderer = glGetString(GL_RENDERER);
    auto version = glGetString(GL_VERSION);
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version: %s\n", version);

    // glEnable(GL_DEBUG_OUTPUT);
    // glDebugMessageCallback(message_callback, 0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    auto vs = load_shader("src/vertex.glsl", GL_VERTEX_SHADER);
    auto gs = load_shader("src/geometry.glsl", GL_GEOMETRY_SHADER);
    auto fs = load_shader("src/fragment.glsl", GL_FRAGMENT_SHADER);

    auto shader_program = glCreateProgram();
    glAttachShader(shader_program, vs);
    glAttachShader(shader_program, gs);
    glAttachShader(shader_program, fs);
    glLinkProgram(shader_program);

    Sphere spheres[]{
        {
            .center = vec3(0.0, 0.5, 0.0),
            .color = vec3(1.0, 0.0, 0.0),
            .radius = 0.2,
        },
        {
            .center = vec3(0.5, 0.0, 0.0),
            .color = vec3(0.0, 1.0, 0.0),
            .radius = 0.15,
        },
        {
            .center = vec3(0.0, -0.5, 0.0),
            .color = vec3(0.0, 0.0, 1.0),
            .radius = 0.1,
        },
        {
            .center = vec3(-0.5, 0.0, 0.0),
            .color = vec3(1.0, 1.0, 1.0),
            .radius = 0.15,
        },
    };

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(spheres), spheres, GL_DYNAMIC_DRAW);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    auto centerAttrib = glGetAttribLocation(shader_program, "center");
    glEnableVertexAttribArray(centerAttrib);
    glVertexAttribPointer(centerAttrib, sizeof(Sphere::center) / sizeof(float),
                          GL_FLOAT, GL_FALSE, sizeof(Sphere),
                          (void*)offsetof(Sphere, center));

    auto colorAttrib = glGetAttribLocation(shader_program, "color");
    glEnableVertexAttribArray(colorAttrib);
    glVertexAttribPointer(colorAttrib, sizeof(Sphere::color) / sizeof(float),
                          GL_FLOAT, GL_FALSE, sizeof(Sphere),
                          (void*)offsetof(Sphere, color));

    auto radiusAttrib = glGetAttribLocation(shader_program, "radius");
    glEnableVertexAttribArray(radiusAttrib);
    glVertexAttribPointer(radiusAttrib, sizeof(Sphere::radius) / sizeof(float),
                          GL_FLOAT, GL_FALSE, sizeof(Sphere),
                          (void*)offsetof(Sphere, radius));

    auto viewUniform = glGetUniformLocation(shader_program, "model_view");
    auto projectionUniform = glGetUniformLocation(shader_program, "projection");

    auto rot_mat =
        glm::rotate(glm::mat4(1.0f), glm::radians(1.0f), vec3(0, 0, 1));

    auto camera_rot =
        glm::rotate(glm::mat4(1.0f), glm::radians(0.5f), vec3(0, 1, 0));

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shader_program);
        camera.transform_position(camera_rot);
        auto view = camera.view();
        auto projection = camera.projection();
        glUniformMatrix4fv(viewUniform, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionUniform, 1, GL_FALSE,
                           glm::value_ptr(projection));
        glBindVertexArray(vao);
        for (auto& s : spheres) {
            s.center = vec3(glm::vec4(s.center, 1.0) * rot_mat);
        }
        glBufferData(GL_ARRAY_BUFFER, sizeof(spheres), spheres,
                     GL_DYNAMIC_DRAW);
        glDrawArrays(GL_POINTS, 0, sizeof(spheres) / sizeof(Sphere));
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
