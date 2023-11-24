#include "camera.hpp"
#include "imgui.h"
#include "shader.hpp"
#include "context.hpp"
#include <GL/glew.h>
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

using glm::vec3;
using glm::vec4;

#include "common/particle.glsl"
#include "common/globals.glsl"

int main(void) {
    Context::init(1280, 720, "fluidsim");

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
                               .vertex_file(for_neighbor)
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

    ComputeShader clear_keys = ComputeShader::builder()
                             .compute_source(version)
                             .compute_file(globals)
                             .compute_file(globals_layout)
                             .compute_file("src/shader/sph/clear_keys.glsl")
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

    ComputeShader prefix_sum = ComputeShader::builder()
                             .compute_source(version)
                             .compute_file(globals)
                             .compute_file(globals_layout)
                             .compute_file("src/shader/sph/prefix_sum.glsl")
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
    glNamedBufferData(globals_ssbo, sizeof(Globals), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, globals_ssbo);

    auto camera = OrbitingCamera(vec3(0, 0, 0), 30, 0, 0);
    Globals G;
    G.gravity = vec4(0, -20, 0, 0);
    G.low_bound = vec3(-30.5, -17, 0);
    G.object_count = 25000;
    G.high_bound = vec3(30.5, 17, 0);
    G.particle_size = 0.1;
    G.smoothing_radius = 1.0;
    G.target_density = 15;
    G.pressure_multiplier = 500;
    G.collision_multiplier = 0.9;
    G.key_count = 5000;
    G.selected_index = 0;
    G.visualization = VISUALIZATION_DENSITY;
    G.density_color_multiplier = 1.0;

    GLuint input_particles;
    GLuint output_particles;
    glGenBuffers(1, &input_particles);
    glGenBuffers(1, &output_particles);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, input_particles);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, output_particles);

    GLuint input_keys;
    GLuint output_keys;
    glGenBuffers(1, &input_keys);
    glGenBuffers(1, &output_keys);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, input_keys);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, output_keys);

    bool object_buffer_regenerate = true;
    uint prev_object_count = 0;
    uint object_buffer_size = 0;

    bool key_buffer_regenerate = true;
    uint key_buffer_size = 0;

    bool paused = true;
    auto prev_frame = std::chrono::steady_clock::now();

    Context::loop([&](){
        auto current_frame = std::chrono::steady_clock::now();
        G.delta_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                          current_frame - prev_frame)
                          .count() /
                      1000.0f;
        prev_frame = current_frame;
        ImGui::Begin("Settings");
        ImGui::Text("FPS: %2.2f", ImGui::GetIO().Framerate);
        if(ImGui::SliderInt("Particle count", (int*)&G.object_count, 1, 100000)) {
            object_buffer_regenerate = true;
        }
        if(ImGui::Button("Restart")) {
            object_buffer_regenerate = true;
            prev_object_count = 0;
        }
        if(ImGui::SliderInt("Key count", (int*)&G.key_count, 10, 100000)) {
            key_buffer_regenerate = true;
        }
        ImGui::Checkbox("Pause", &paused);
        ImGui::SeparatorText("Camera settings");
        ImGui::DragFloat("Camera yaw", &camera.yaw, 0.2, 0, 360);
        ImGui::DragFloat("Camera pitch", &camera.pitch, 0.1, -89.999, 89.999);
        ImGui::DragFloat("Camera distance", &camera.distance, 0.02, 1, 100);
        ImGui::SeparatorText("Generic physics settings");
        ImGui::DragFloat3("Gravity", glm::value_ptr(G.gravity), 0.1, -100, 100);
        ImGui::DragFloat3("Box high bound", glm::value_ptr(G.high_bound), 0.02, 0, 60);
        ImGui::DragFloat3("Box low bound", glm::value_ptr(G.low_bound), 0.02, -60, 0);
        ImGui::SeparatorText("SPH settings");
        ImGui::DragFloat("Smoothing radius", &G.smoothing_radius, 0.001, 0.01, 10);
        ImGui::DragFloat("Target density", &G.target_density, 0.01, 0.01, 100);
        ImGui::DragFloat("Pressure multiplier", &G.pressure_multiplier, 0.1, 0.01, 1000);
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
        if(ImGui::RadioButton("Visualize cell key (expected)",
                              G.visualization == VISUALIZATION_CELL_KEY_EXPECTED)
        ) {
            G.visualization = VISUALIZATION_CELL_KEY_EXPECTED;
        }
        if(ImGui::RadioButton("Visualize cell key (actual)",
                              G.visualization == VISUALIZATION_CELL_KEY_ACTUAL)
        ) {
            G.visualization = VISUALIZATION_CELL_KEY_ACTUAL;
        }
        if(ImGui::RadioButton("Visualize speed",
                              G.visualization == VISUALIZATION_SPEED)
        ) {
            G.visualization = VISUALIZATION_SPEED;
        }
        ImGui::End();
        glNamedBufferSubData(globals_ssbo, 0, sizeof(G), &G);

        if (object_buffer_regenerate) {
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
            object_buffer_regenerate = false;
        }

        if (key_buffer_regenerate) {
            if (key_buffer_size < G.key_count + 1) {
                glNamedBufferData(
                    input_keys, (G.key_count + 1) * sizeof(uint),
                    NULL, GL_DYNAMIC_COPY
                );
                glNamedBufferData(
                    output_keys, (G.key_count + 1) * sizeof(uint),
                    NULL, GL_DYNAMIC_COPY
                );
                key_buffer_size = G.key_count + 1;
            }
            key_buffer_regenerate = false;
        }

        if (G.key_count > 0) {
            // zero fill input key counts
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, input_keys);
            clear_keys.dispatch_executions(G.key_count + 1);

            // count keys
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, input_particles);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, output_particles);
            predict_and_hash.dispatch_executions(G.object_count);
            std::swap(input_particles, output_particles);

            // calculate key indicies
            for (uint offset = 1; offset < G.key_count + 1; offset *= 2) {
                glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, input_keys);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, output_keys);
                prefix_sum.uniform("offset", offset);
                prefix_sum.dispatch_executions(G.key_count + 1);
                std::swap(input_keys, output_keys);
            }

            // bucket sort using key indicies
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, input_keys);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, input_particles);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, output_particles);
            bucket_sort.dispatch_executions(G.object_count);
            std::swap(input_particles, output_particles);
        }
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
    });

    Context::uninit();
    return 0;
}
