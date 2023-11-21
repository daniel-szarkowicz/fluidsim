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
#include "common/globals.glsl"

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
    const char* globals = "src/common/globals.glsl";
    const char* globals_layout = "src/shader/globals_layout.glsl";
    const char* hash = "src/shader/hash.glsl";
    const char* kernel = "src/shader/sph/kernel.glsl";
    const char* for_neighbor = "src/shader/for_neighbor.glsl";

    GraphicsShader particle_shader = GraphicsShader::builder()
                               .vertex_source(version)
                               .vertex_file(particle)
                               .vertex_file(globals)
                               .vertex_file(globals_layout)
                               .vertex_file(hash)
                               .vertex_file("src/shader/particle_vertex.glsl")
                               .geometry_source(version)
                               .geometry_file("src/shader/particle_geometry.glsl")
                               .fragment_source(version)
                               .fragment_file("src/shader/particle_fragment.glsl")
                               .build();

    GraphicsShader box_shader = GraphicsShader::builder()
                            .vertex_source(version)
                            .vertex_file(globals)
                            .vertex_file(globals_layout)
                            .vertex_file("src/shader/box_vertex.glsl")
                            .fragment_source(version)
                            .fragment_file("src/shader/box_fragment.glsl")
                            .build();

    ComputeShader generate_particles = ComputeShader::builder()
                             .compute_source(version)
                             .compute_file(particle)
                             .compute_file(globals)
                             .compute_file(globals_layout)
                             .compute_file(hash)
                             .compute_file("src/shader/generate_particles.glsl")
                             .build();

    ComputeShader predict_and_hash = ComputeShader::builder()
                             .compute_source(version)
                             .compute_file(particle)
                             .compute_file(globals)
                             .compute_file(globals_layout)
                             .compute_file(hash)
                             .compute_file(kernel)
                             .compute_file("src/shader/sph/predict_and_hash.glsl")
                             .build();

    ComputeShader bucket_sort = ComputeShader::builder()
                             .compute_source(version)
                             .compute_file(particle)
                             .compute_file(globals)
                             .compute_file(globals_layout)
                             .compute_file(hash)
                             .compute_file(kernel)
                             .compute_file("src/shader/sph/bucket_sort.glsl")
                             .build();
    ComputeShader density = ComputeShader::builder()
                             .compute_source(version)
                             .compute_file(particle)
                             .compute_file(globals)
                             .compute_file(globals_layout)
                             .compute_file(hash)
                             .compute_file(kernel)
                             .compute_file(for_neighbor)
                             .compute_file("src/shader/sph/density.glsl")
                             .build();

    ComputeShader pressure_force = ComputeShader::builder()
                             .compute_source(version)
                             .compute_file(particle)
                             .compute_file(globals)
                             .compute_file(globals_layout)
                             .compute_file(hash)
                             .compute_file(kernel)
                             .compute_file(for_neighbor)
                             .compute_file("src/shader/sph/pressure_force.glsl")
                             .build();

    ComputeShader update_position = ComputeShader::builder()
                             .compute_source(version)
                             .compute_file(particle)
                             .compute_file(globals)
                             .compute_file(globals_layout)
                             .compute_file(hash)
                             .compute_file(kernel)
                             .compute_file("src/shader/sph/update_position.glsl")
                             .build();

    ComputeShader compute_pipeline[] = {
        density,
        pressure_force,
        update_position,
    };

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

    GLuint globals_ssbo;
    glGenBuffers(1, &globals_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, globals_ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, globals_ssbo);

    auto camera = OrbitingCamera(vec3(0, 0, 0), 30, 0, 0);
    Globals G;
    G.gravity = vec4(0, 0, 0, 0);
    G.low_bound = vec3(-15, -8, -15);
    G.object_count = 5000;
    G.high_bound = vec3(15, 8, 15);
    G.particle_size = 0.1;
    G.smoothing_radius = 1.0;
    G.target_density = 1.0;
    G.pressure_multiplier = 1.0;
    G.collision_multiplier = 0.9;
    // ez a szimulációhoz lehet nagyobb, de így jobb a vizualizáció
    G.key_count = 8*1024/4; // 8 KB / 4 B
    G.selected_index = 0;
    G.visualization = VISUALIZATION_DENSITY;
    G.density_color_multiplier = 1.0;

    GLuint input_particles;
    GLuint output_particles;
    glGenBuffers(1, &input_particles);
    glGenBuffers(1, &output_particles);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, input_particles);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, output_particles);

    GLuint key_counters;
    glGenBuffers(1, &key_counters);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, key_counters);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, key_counters);

    bool paused = true;
    bool generate = true;
    uint prev_object_count = 0;
    uint object_buffer_size = 0;
    auto prev_frame = std::chrono::steady_clock::now();

    while (!glfwWindowShouldClose(window)) {
        auto current_frame = std::chrono::steady_clock::now();
        G.delta_time = std::chrono::duration_cast<std::chrono::milliseconds>(
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
        if(ImGui::SliderInt("Particle count", (int*)&G.object_count, 1, 10000)) {
            generate = true;
        }
        if(ImGui::Button("Restart")) {
            generate = true;
            prev_object_count = 0;
        }
        ImGui::SliderInt("Key count", (int*)&G.key_count, 1, 10000); 
        ImGui::Checkbox("Pause", &paused);
        ImGui::SeparatorText("Camera settings");
        ImGui::DragFloat("Camera yaw", &camera.yaw, 0.2, 0, 360);
        ImGui::DragFloat("Camera pitch", &camera.pitch, 0.1, -89.999, 89.999);
        ImGui::DragFloat("Camera distance", &camera.distance, 0.02, 1, 100);
        ImGui::SeparatorText("Generic physics settings");
        ImGui::DragFloat3("Gravity", glm::value_ptr(G.gravity), 0.01, -10, 10);
        ImGui::DragFloat3("Box high bound", glm::value_ptr(G.high_bound), 0.02, 0, 30);
        ImGui::DragFloat3("Box low bound", glm::value_ptr(G.low_bound), 0.02, -30, 0);
        ImGui::SeparatorText("SPH settings");
        ImGui::DragFloat("Smoothing radius", &G.smoothing_radius, 0.001, 0.01, 10);
        ImGui::DragFloat("Targed density", &G.target_density, 0.001, 0.01, 10);
        ImGui::DragFloat("Pressure multiplier", &G.pressure_multiplier, 0.001, 0.01, 10);
        ImGui::DragFloat("Collision multiplier", &G.collision_multiplier, 0.01, 0.1, 1);
        ImGui::SeparatorText("Visualization settings");
        ImGui::DragFloat("Particle size", &G.particle_size, 0.001, 0.01, 10);
        ImGui::DragFloat("Density color multiplier", &G.density_color_multiplier, 0.01, 0.01, 10);
        ImGui::DragInt("Selected particle (cell key)", (int*)&G.selected_index, 0.1, 0, G.object_count-1);
        if(ImGui::RadioButton("Visualize density",
                              G.visualization == VISUALIZATION_DENSITY)
        ) {
            G.visualization = VISUALIZATION_DENSITY;
        }
        if(ImGui::RadioButton("Visualize cell key",
                              G.visualization == VISUALIZATION_CELL_KEY)
        ) {
            G.visualization = VISUALIZATION_CELL_KEY;
        }
        if(ImGui::RadioButton("Visualize key index",
                              G.visualization == VISUALIZATION_KEY_INDEX)
        ) {
            G.visualization = VISUALIZATION_KEY_INDEX;
        }
        ImGui::End();
        glNamedBufferData(globals_ssbo, sizeof(G), &G, GL_DYNAMIC_DRAW);

        if (generate) {
            if (object_buffer_size < G.object_count) {
                glNamedBufferData(
                    output_particles, G.object_count * sizeof(Particle),
                    NULL, GL_DYNAMIC_COPY
                );
            }
            if (prev_object_count < G.object_count) {
                glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, input_particles);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, output_particles);
                generate_particles.uniform("prev_object_count", prev_object_count);
                generate_particles.dispatch_executions(
                    G.object_count
                );
                std::swap(input_particles, output_particles);
            }
            prev_object_count = G.object_count;
            if (object_buffer_size < G.object_count) {
                glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
                glNamedBufferData(
                    output_particles, G.object_count * sizeof(Particle),
                    NULL, GL_DYNAMIC_COPY
                );
                object_buffer_size = G.object_count;
            }
            generate = false;
        }

        auto key_counters_data = std::vector<uint>(G.key_count, 0);
        glNamedBufferData(
            key_counters,
            key_counters_data.size() * sizeof(GLuint),
            &key_counters_data[0], GL_DYNAMIC_READ
        );
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, input_particles);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, output_particles);
        predict_and_hash.dispatch_executions(G.object_count);
        std::swap(input_particles, output_particles);
        glGetNamedBufferSubData(
            key_counters, 0,
            key_counters_data.size() * sizeof(GLuint),
            &key_counters_data[0]
        );
        auto key_indicies = std::vector<uint>(G.key_count + 1, 0);
        for (uint i = 1; i < G.key_count + 1; ++i) {
            key_indicies[i] = key_indicies[i-1] + key_counters_data[i-1];
        }
        glNamedBufferData(
            key_counters,
            key_indicies.size() * sizeof(GLuint),
            &key_indicies[0], GL_DYNAMIC_DRAW
        );
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, input_particles);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, output_particles);
        bucket_sort.dispatch_executions(G.object_count);
        std::swap(input_particles, output_particles);
        if (!paused) {

            for (auto shader : compute_pipeline) {
                glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, input_particles);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, output_particles);
                shader.dispatch_executions(G.object_count);
                std::swap(input_particles, output_particles);
            }
        }

        particle_shader.use();
        particle_shader.uniform("view", camera.view());
        particle_shader.uniform("projection", camera.projection());
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, input_particles);
        glBindVertexArray(empty_vao);
        glDrawArraysInstanced(GL_POINTS, 0, 1, G.object_count);

        box_shader.use();
        box_shader.uniform("view_projection", camera.view_projection());
        glBindVertexArray(box_vao);
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
