#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#include "camera.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>
#include <stdio.h>
#include <string>

using glm::vec3;
using glm::vec4;

struct Sphere {
    vec4 center;
    vec4 velocity;
    vec4 color;
    float radius;
    float _padding[3];
};

void GLAPIENTRY message_callback(GLenum source, GLenum type, GLuint id,
                                 GLenum severity, GLsizei length,
                                 const GLchar* message, const void* userParam) {
    (void)source;
    (void)id;
    (void)length;
    (void)userParam;
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

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    (void)window;
    glViewport(0, 0, width, height);
}

int main(void) {
    if (!glfwInit()) {
        fprintf(stderr, "GLFW init error\n");
        return 1;
    }

    auto window = glfwCreateWindow(1280, 720, "fluidsim", NULL, NULL);
    if (!window) {
        fprintf(stderr, "GLFW window error\n");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 430");

    glewInit();
    auto renderer = glGetString(GL_RENDERER);
    auto version = glGetString(GL_VERSION);
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version: %s\n", version);

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(message_callback, 0);
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

    auto cs = load_shader("src/compute.glsl", GL_COMPUTE_SHADER);
    auto compute_program = glCreateProgram();
    glAttachShader(compute_program, cs);
    glLinkProgram(compute_program);

    Sphere spheres[]{
        {
            .center = vec4(0.0, 0.5, 0.5, 1.0),
            .velocity = vec4(glm::ballRand(0.02), 0.0),
            .color = vec4(1.0, 0.0, 0.0, 1.0),
            .radius = 0.2,
        },
        {
            .center = vec4(0.5, 0.0, 0.5, 1.0),
            .velocity = vec4(glm::ballRand(0.02), 0.0),
            .color = vec4(0.0, 1.0, 0.0, 1.0),
            .radius = 0.15,
        },
        {
            .center = vec4(0.0, -0.5, 0.5, 1.0),
            .velocity = vec4(glm::ballRand(0.02), 0.0),
            .color = vec4(0.0, 0.0, 1.0, 1.0),
            .radius = 0.1,
        },
        {
            .center = vec4(-0.5, 0.0, 0.5, 1.0),
            .velocity = vec4(glm::ballRand(0.02), 0.0),
            .color = vec4(1.0, 1.0, 1.0, 1.0),
            .radius = 0.15,
        },
    };

    GLuint ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(spheres), spheres,
                 GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo);

    auto viewUniform = glGetUniformLocation(shader_program, "view");
    auto projectionUniform = glGetUniformLocation(shader_program, "projection");

    auto camera = OrbitingCamera(vec3(0, 0, 0), 5, 0, 0);
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Settings");
        ImGui::SliderFloat("Camera yaw", &camera.yaw, 0, 360);
        ImGui::SliderFloat("Camera pitch", &camera.pitch, -89.999, 89.999);
        ImGui::DragFloat("Camera distance", &camera.distance, 0.02, 1, 25);
        ImGui::End();
        glUseProgram(compute_program);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo);
        glDispatchCompute(4, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glUseProgram(shader_program);
        glUniformMatrix4fv(viewUniform, 1, GL_FALSE,
                           glm::value_ptr(camera.view()));
        glUniformMatrix4fv(projectionUniform, 1, GL_FALSE,
                           glm::value_ptr(camera.projection()));
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo);
        glDrawArraysInstanced(GL_POINTS, 0, 1, 4);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
