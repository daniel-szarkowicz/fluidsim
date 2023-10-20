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

struct Sphere {
    vec4 center;         // 128 bits
    vec4 velocity;       // 128 bits
    vec4 color;          // 128 bits
    GLfloat radius;      // 32 bits
    GLuint cell_hash;    // 32 bits
    GLuint obj_id;       // 32 bits
    GLubyte _padding[4]; // 32 bits
};

// https://www.shadertoy.com/view/WttXWX
GLuint hash(GLuint x) {
    x ^= x >> 17;
    x *= 0xed5ad4bbU;
    x ^= x >> 11;
    x *= 0xac4c1b51U;
    x ^= x >> 15;
    x *= 0x31848babU;
    x ^= x >> 14;
    return x;
}

GLuint cell_hash(vec4 position) {
    vec4 cell_pos = glm::floor(position / 1.0f);
    GLuint cell_hash = hash(glm::floatBitsToUint(cell_pos.x));
    cell_hash = hash(cell_hash + glm::floatBitsToUint(cell_pos.y));
    cell_hash = hash(cell_hash + glm::floatBitsToUint(cell_pos.z));
    return cell_hash;
}

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
    auto renderer = glGetString(GL_RENDERER);
    auto version = glGetString(GL_VERSION);
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version: %s\n", version);

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(message_callback, 0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    Shader sphere_shader = Shader::builder()
                               .vertex_file("src/shader/sphere_vertex.glsl")
                               .geometry_file("src/shader/sphere_geometry.glsl")
                               .fragment_file("src/shader/sphere_fragment.glsl")
                               .build();

    Shader box_shader = Shader::builder()
                            .vertex_file("src/shader/box_vertex.glsl")
                            .fragment_file("src/shader/box_fragment.glsl")
                            .build();

    Shader compute_shader =
        Shader::builder().compute_file("src/shader/compute.glsl").build();

    printf("bitonic\n");
    Shader bitonic_sort_shader =
        Shader::builder().compute_file("src/shader/bitonic.glsl").build();

    // TODO: move init to a compute shader
    std::vector<Sphere> spheres(10000);
    for (size_t i = 0; i < spheres.size(); ++i) {
        spheres[i] = Sphere{
            .center =
                vec4(glm::linearRand(-5.0f, 5.0f), glm::linearRand(-5.0f, 5.0f),
                     glm::linearRand(-5.0f, 5.0f), 1),
            .velocity = glm::vec4(glm::ballRand(20.0f), 0.0f) *
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
        spheres[i].cell_hash = cell_hash(spheres[i].center);
        spheres[i].obj_id = i;
    }
    spheres[0].radius = 0.25;
    spheres[1].radius = 0.01;

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
    glBufferData(GL_SHADER_STORAGE_BUFFER, spheres.size() * sizeof(Sphere),
                 &spheres[0], GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, spheres.size() * sizeof(Sphere),
                 &spheres[0], GL_DYNAMIC_DRAW);

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
            paused = true;
            ssbo_flip = 1 - ssbo_flip;
            GLint compute_work_groups[3];

            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            bitonic_sort_shader.use();
            glGetProgramiv(bitonic_sort_shader.program_id,
                           GL_COMPUTE_WORK_GROUP_SIZE, compute_work_groups);
            bitonic_sort_shader.uniform("object_count", (GLuint)spheres.size());
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo[ssbo_flip]);
            glDispatchCompute((spheres.size() + compute_work_groups[0] - 1) /
                                  compute_work_groups[0],
                              1, 1);

            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            compute_shader.use();
            glGetProgramiv(compute_shader.program_id,
                           GL_COMPUTE_WORK_GROUP_SIZE, compute_work_groups);
            compute_shader.uniform("gravity", gravity);
            compute_shader.uniform("low_bound", low_bound);
            compute_shader.uniform("high_bound", high_bound);
            compute_shader.uniform("dt", delta);
            compute_shader.uniform("collision_multiplier",
                                   collision_multiplier);
            compute_shader.uniform("object_count", (GLuint)spheres.size());

            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo[ssbo_flip]);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssbo[1 - ssbo_flip]);
            glDispatchCompute((spheres.size() + compute_work_groups[0] - 1) /
                                  compute_work_groups[0],
                              1, 1);
        }

        sphere_shader.use();
        sphere_shader.uniform("view", camera.view());
        sphere_shader.uniform("projection", camera.projection());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo[ssbo_flip]);
        glBindVertexArray(empty_vao);
        glDrawArraysInstanced(GL_POINTS, 0, 1, spheres.size());

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
