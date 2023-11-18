#include "camera.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "shader.hpp"
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

#include "common/particle.glsl"

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
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("OpenGL version: %s\n", glGetString(GL_VERSION));

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(message_callback, 0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    const char* version = "#version 430";
    const char* particle = "src/common/particle.glsl";
    Shader particle_shader = Shader::builder()
                               .vertex_source(version)
                               .vertex_file(particle)
                               .vertex_file("src/shader/particle_vertex.glsl")
                               .geometry_source(version)
                               .geometry_file("src/shader/particle_geometry.glsl")
                               .fragment_source(version)
                               .fragment_file("src/shader/particle_fragment.glsl")
                               .build();
    printf("particle_shader\n");

    Shader box_shader = Shader::builder()
                            .vertex_source(version)
                            .vertex_file("src/shader/box_vertex.glsl")
                            .fragment_source(version)
                            .fragment_file("src/shader/box_fragment.glsl")
                            .build();

    // Shader compute_shader = Shader::builder()
    //                             .compute_source(version)
    //                             .compute_file(particle)
    //                             .compute_file("src/shader/compute.glsl")
    //                             .build();
    Shader density = Shader::builder()
                             .compute_source(version)
                             .compute_file(particle)
                             .compute_file("src/shader/sph/kernel.glsl")
                             .compute_file("src/shader/sph/density.glsl")
                             .build();

    std::vector<Particle> particles(8000);
    for (size_t i = 0; i < particles.size(); ++i) {
        particles[i] = Particle{
            // .position =
            //     vec4(glm::linearRand(-3.0f, 3.0f), glm::linearRand(-3.0f, 3.0f),
            //          glm::linearRand(-3.0f, 3.0f), 1),
            // .velocity = glm::vec4(glm::ballRand(20.0f), 0.0f) *
            //             glm::linearRand(0.5f, 1.0f),
            .position =
                vec4(glm::linearRand(-15.0f, 15.0f),
                glm::linearRand(-8.0f, 8.0f), 0, 1),
            // .velocity = glm::vec4(glm::circularRand(4.0f), 0, 0.0f) *
            //             glm::linearRand(0.5f, 1.0f),
            .velocity = vec4(0, 0, 0, 0),
            .mass = 1.0f,
        };
    }

    GLuint empty_vao;
    glCreateVertexArrays(1, &empty_vao);

    GLuint box_vao;
    glCreateVertexArrays(1, &box_vao);
    glBindVertexArray(box_vao);
    GLuint box_vbo;
    glGenBuffers(1, &box_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, box_vbo);
    vec3 points[] = {
        vec3(-1, 1, -1),  vec3(-1, 1, 1),   vec3(-1, 1, -1),  vec3(1, 1, -1),
        vec3(1, 1, 1),    vec3(-1, 1, 1),   vec3(1, 1, 1),    vec3(1, 1, -1),
        vec3(-1, -1, -1), vec3(-1, -1, 1),  vec3(-1, -1, -1), vec3(1, -1, -1),
        vec3(1, -1, 1),   vec3(-1, -1, 1),  vec3(1, -1, 1),   vec3(1, -1, -1),
        vec3(-1, 1, -1),  vec3(-1, -1, -1), vec3(1, 1, -1),   vec3(1, -1, -1),
        vec3(-1, 1, 1),   vec3(-1, -1, 1),  vec3(1, 1, 1),    vec3(1, -1, 1),
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    GLuint ssbo[2];
    glGenBuffers(2, ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, particles.size() * sizeof(Particle),
                 &particles[0], GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, particles.size() * sizeof(Particle),
                 &particles[0], GL_DYNAMIC_DRAW);

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
            density.use();
            GLint compute_work_groups[3];
            glGetProgramiv(density.program_id,
                           GL_COMPUTE_WORK_GROUP_SIZE, compute_work_groups);

            density.uniform("object_count", (GLuint)particles.size());

            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo[ssbo_flip]);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssbo[1 - ssbo_flip]);
            glDispatchCompute((particles.size() + compute_work_groups[0] - 1) /
                                  compute_work_groups[0],
                              1, 1);
        }

        particle_shader.use();
        particle_shader.uniform("view", camera.view());
        particle_shader.uniform("projection", camera.projection());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo[ssbo_flip]);
        glBindVertexArray(empty_vao);
        glDrawArraysInstanced(GL_POINTS, 0, 1, particles.size());

        box_shader.use();
        box_shader.uniform("view_projection", camera.view_projection());
        box_shader.uniform("low_bound", low_bound);
        box_shader.uniform("high_bound", high_bound);
        glBindVertexArray(box_vao);
        glBindBuffer(GL_ARRAY_BUFFER, box_vbo);
        glDrawArrays(GL_LINES, 0, 24);

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
