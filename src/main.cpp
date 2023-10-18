#include "camera.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <chrono>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>
#include <stdio.h>
#include <string>
#include <vector>

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

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
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

    std::vector<Sphere> spheres(12800);
    for (size_t i = 0; i < spheres.size(); ++i) {
        spheres[i] = Sphere{
            .center =
                vec4(glm::linearRand(-3.0f, 3.0f), glm::linearRand(-3.0f, 3.0f),
                     glm::linearRand(-3.0f, 3.0f), 1),
            .velocity = glm::vec4(glm::ballRand(4.0f), 0.0f) *
                        glm::linearRand(0.5f, 1.0f),
            // .center =
            //     vec4(glm::linearRand(-3.0f, 3.0f),
            //     glm::linearRand(-3.0f, 3.0f), 0, 1),
            // .velocity = glm::vec4(glm::circularRand(4.0f), 0, 0.0f) *
            //             glm::linearRand(0.5f, 1.0f),
            .color =
                vec4(glm::linearRand(0.1f, 1.0f), glm::linearRand(0.1f, 1.0f),
                     glm::linearRand(0.1f, 1.0f), 1.0f),
            .radius = glm::linearRand(0.05f, 0.2f),
        };
    }
    printf("%zu\n", spheres.size());

    GLuint vao;
    glCreateVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint ssbo[2];
    glGenBuffers(2, ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, spheres.size() * sizeof(Sphere),
                 &spheres[0], GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, spheres.size() * sizeof(Sphere),
                 &spheres[0], GL_DYNAMIC_DRAW);

    auto viewUniform = glGetUniformLocation(shader_program, "view");
    auto projectionUniform = glGetUniformLocation(shader_program, "projection");
    auto gravityUniform = glGetUniformLocation(compute_program, "gravity");
    auto lowBoundUniform = glGetUniformLocation(compute_program, "low_bound");
    auto highBoundUniform = glGetUniformLocation(compute_program, "high_bound");
    auto dtUniform = glGetUniformLocation(compute_program, "dt");
    auto collisionMultiplierUniform =
        glGetUniformLocation(compute_program, "collision_multiplier");

    auto camera = OrbitingCamera(vec3(0, 0, 0), 30, 0, 0);
    vec4 gravity = vec4(0, -8, 0, 0);
    vec3 low_bound = vec3(-15, -8, -15);
    vec3 high_bound = vec3(15, 8, 15);
    float collision_multiplier = 0.95;
    uint8_t ssbo_flip = 0;
    bool paused = false;
    auto prev_frame = std::chrono::steady_clock::now();

    while (!glfwWindowShouldClose(window)) {
        auto current_frame = std::chrono::steady_clock::now();
        float delta = std::chrono::duration_cast<std::chrono::milliseconds>(
                          current_frame - prev_frame)
                          .count() /
                      1000.0f;
        prev_frame = current_frame;
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Settings");
        ImGui::Text("FPS: %2.2f", ImGui::GetIO().Framerate);
        ImGui::Checkbox("Pause", &paused);
        ImGui::Separator();
        ImGui::DragFloat("Camera yaw", &camera.yaw, 0.2, 0, 360);
        ImGui::DragFloat("Camera pitch", &camera.pitch, 0.1, -89.999, 89.999);
        ImGui::DragFloat("Camera distance", &camera.distance, 0.02, 1, 100);
        ImGui::Separator();
        ImGui::DragFloat3("Gravity", glm::value_ptr(gravity), 0.01, -10, 10);
        ImGui::DragFloat3("Box high bound", glm::value_ptr(high_bound), 0.02, 0,
                          30);
        ImGui::DragFloat3("Box low bound", glm::value_ptr(low_bound), 0.02, -30,
                          0);
        ImGui::DragFloat("Collision multiplier", &collision_multiplier, 0.001,
                         0, 1);
        ImGui::End();
        if (!paused) {
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            ssbo_flip = 1 - ssbo_flip;
            glUseProgram(compute_program);
            glUniform4fv(gravityUniform, 1, glm::value_ptr(gravity));
            glUniform3fv(lowBoundUniform, 1, glm::value_ptr(low_bound));
            glUniform3fv(highBoundUniform, 1, glm::value_ptr(high_bound));
            glUniform1f(dtUniform, delta);
            glUniform1f(collisionMultiplierUniform, collision_multiplier);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo[ssbo_flip]);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssbo[1 - ssbo_flip]);
            glDispatchCompute(spheres.size() / 32, 1, 1);
        }
        glUseProgram(shader_program);
        glUniformMatrix4fv(viewUniform, 1, GL_FALSE,
                           glm::value_ptr(camera.view()));
        glUniformMatrix4fv(projectionUniform, 1, GL_FALSE,
                           glm::value_ptr(camera.projection()));
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo[ssbo_flip]);
        glDrawArraysInstanced(GL_POINTS, 0, 1, spheres.size());
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
