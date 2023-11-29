#include "camera.hpp"
#include "imgui.h"
#include "shader.hpp"
#include "context.hpp"
#include <GL/glew.h>
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "ssbo.hpp"

using glm::vec3;
using glm::ivec3;

#include "common/particle.glsl"
#include "common/globals.glsl"

int main(void) {
    Context::init(1280, 720, "fluidsim");

    const char* version = "#version 430";
    const char* particle = "src/common/particle.glsl";
    const char* globals = "src/common/globals.glsl";
    const char* particles_readonly_layout = "src/shader/particles_readonly_layout.glsl";
    const char* particles_double_layout = "src/shader/particles_double_layout.glsl";
    const char* globals_layout = "src/shader/globals_layout.glsl";
    const char* keys_readonly_layout = "src/shader/keys_readonly_layout.glsl";
    const char* hash = "src/shader/hash.glsl";
    const char* kernel = "src/shader/sph/kernel.glsl";
    const char* for_neighbor = "src/shader/for_neighbor.glsl";
    const char* compute_layout = "src/shader/compute_layout.glsl";
    const char* keys_readwrite_layout = "src/shader/keys_readwrite_layout.glsl";
    const char* keys_double_layout = "src/shader/keys_double_layout.glsl";

    GraphicsShader particle_shader = GraphicsShader::builder()
        .vertex_source(version)
        .vertex_file(particle)
        .vertex_file(globals)
        .vertex_file(particles_readonly_layout)
        .vertex_file(globals_layout)
        .vertex_file(keys_readonly_layout)
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
        .compute_file(compute_layout)
        .compute_file(particle)
        .compute_file(globals)
        .compute_file(particles_double_layout)
        .compute_file(globals_layout)
        .compute_file(hash)
        .compute_file("src/shader/generate_particles.glsl")
        .build();

    ComputeShader clear_keys = ComputeShader::builder()
        .compute_source(version)
        .compute_file(compute_layout)
        .compute_file(globals)
        .compute_file(globals_layout)
        .compute_file(keys_readwrite_layout)
        .compute_file("src/shader/sph/clear_keys.glsl")
        .build();

    ComputeShader predict_and_hash = ComputeShader::builder()
        .compute_source(version)
        .compute_file(compute_layout)
        .compute_file(particle)
        .compute_file(globals)
        .compute_file(particles_double_layout)
        .compute_file(globals_layout)
        .compute_file(keys_readwrite_layout)
        .compute_file(hash)
        .compute_file(kernel)
        .compute_file("src/shader/sph/predict_and_hash.glsl")
        .build();

    ComputeShader prefix_sum = ComputeShader::builder()
        .compute_source(version)
        .compute_file(compute_layout)
        .compute_file(globals)
        .compute_file(globals_layout)
        .compute_file(keys_double_layout)
        .compute_file("src/shader/sph/prefix_sum.glsl")
        .build();

    ComputeShader bucket_sort = ComputeShader::builder()
        .compute_source(version)
        .compute_file(compute_layout)
        .compute_file(particle)
        .compute_file(globals)
        .compute_file(particles_double_layout)
        .compute_file(globals_layout)
        .compute_file(keys_readonly_layout)
        .compute_file(hash)
        .compute_file(kernel)
        .compute_file("src/shader/sph/bucket_sort.glsl")
        .build();

    ComputeShader density = ComputeShader::builder()
        .compute_source(version)
        .compute_file(compute_layout)
        .compute_file(particle)
        .compute_file(globals)
        .compute_file(particles_double_layout)
        .compute_file(globals_layout)
        .compute_file(keys_readonly_layout)
        .compute_file(hash)
        .compute_file(kernel)
        .compute_file(for_neighbor)
        .compute_file("src/shader/sph/density.glsl")
        .build();

    ComputeShader pressure_force = ComputeShader::builder()
        .compute_source(version)
        .compute_file(compute_layout)
        .compute_file(particle)
        .compute_file(globals)
        .compute_file(particles_double_layout)
        .compute_file(globals_layout)
        .compute_file(keys_readonly_layout)
        .compute_file(hash)
        .compute_file(kernel)
        .compute_file(for_neighbor)
        .compute_file("src/shader/sph/pressure_force.glsl")
        .build();

    ComputeShader update_position = ComputeShader::builder()
        .compute_source(version)
        .compute_file(compute_layout)
        .compute_file(particle)
        .compute_file(globals)
        .compute_file(particles_double_layout)
        .compute_file(globals_layout)
        .compute_file(hash)
        .compute_file(kernel)
        .compute_file("src/shader/sph/update_position.glsl")
        .build();

    ComputeShader viscosity = ComputeShader::builder()
        .compute_source(version)
        .compute_file(compute_layout)
        .compute_file(particle)
        .compute_file(globals)
        .compute_file(particles_double_layout)
        .compute_file(globals_layout)
        .compute_file(keys_readonly_layout)
        .compute_file(hash)
        .compute_file(kernel)
        .compute_file(for_neighbor)
        .compute_file("src/shader/sph/viscosity.glsl")
        .build();

    GLuint empty_vao;
    glCreateVertexArrays(1, &empty_vao);

    SSBO box_ssbo = SSBO(9, GL_STATIC_DRAW);
    glm::vec4 points[] = {
        {-1, 1, -1, 1},  {-1, 1, 1, 1},   {-1, 1, -1, 1},  {1, 1, -1, 1},
        {1, 1, 1, 1},    {-1, 1, 1, 1},   {1, 1, 1, 1},    {1, 1, -1, 1},
        {-1, -1, -1, 1}, {-1, -1, 1, 1},  {-1, -1, -1, 1}, {1, -1, -1, 1},
        {1, -1, 1, 1},   {-1, -1, 1, 1},  {1, -1, 1, 1},   {1, -1, -1, 1},
        {-1, 1, -1, 1},  {-1, -1, -1, 1}, {1, 1, -1, 1},   {1, -1, -1, 1},
        {-1, 1, 1, 1},   {-1, -1, 1, 1},  {1, 1, 1, 1},    {1, -1, 1, 1},
    };
    box_ssbo.resize(sizeof(points));
    box_ssbo.set_data(sizeof(points), points);

    SSBO globals_ssbo = SSBO(1, GL_DYNAMIC_DRAW);
    globals_ssbo.resize(sizeof(Globals));

    auto camera = OrbitingCamera(vec3(0, 0, 0), 30, 0, 0);
    Globals G;
    G.gravity = vec3(0, -3, 0);
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
    G.low_bound_cell = ivec3(floor(G.low_bound / G.smoothing_radius));
    G.high_bound_cell = ivec3(floor(G.high_bound / G.smoothing_radius));
    G.grid_size = G.high_bound_cell - G.low_bound_cell + ivec3(1, 1, 1);
    G.sigma_viscosity = 0.3;
    G.near_density_multiplier = 2;

    auto particles = std::make_shared<SSBOPair>(3, 4, GL_DYNAMIC_COPY);

    auto keys = std::make_shared<SSBOPair>(2, 5, GL_DYNAMIC_COPY);

    generate_particles.ssbopairs.insert(particles);
    clear_keys.ssbos.insert(keys->input);
    predict_and_hash.ssbopairs.insert(particles);
    predict_and_hash.ssbos.insert(keys->input);
    prefix_sum.ssbopairs.insert(keys);
    bucket_sort.ssbopairs.insert(particles);
    bucket_sort.ssbos.insert(keys->input);
    density.ssbos.insert(keys->input);
    density.ssbopairs.insert(particles);
    viscosity.ssbos.insert(keys->input);
    viscosity.ssbopairs.insert(particles);
    pressure_force.ssbos.insert(keys->input);
    pressure_force.ssbopairs.insert(particles);
    update_position.ssbopairs.insert(particles);

    bool object_buffer_regenerate = true;
    uint prev_object_count = 0;

    bool key_buffer_regenerate = true;

    bool paused = true;
    auto prev_frame = std::chrono::steady_clock::now();

    Context::loop([&](){
        auto current_frame = std::chrono::steady_clock::now();
        // G.delta_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        //                   current_frame - prev_frame)
        //                   .count() /
        //               1000.0f;
        G.delta_time = 1.0/60.0;
        prev_frame = current_frame;
        ImGui::Begin("Settings");
        ImGui::Text("FPS: %2.2f", ImGui::GetIO().Framerate);
        if(ImGui::SliderInt("Particle count", (int*)&G.object_count, 1, 200000)) {
            object_buffer_regenerate = true;
        }
        if(ImGui::Button("Restart")) {
            object_buffer_regenerate = true;
            prev_object_count = 0;
        }
        uint recommended = std::max(G.grid_size.x * G.grid_size.y * G.grid_size.z, 10);
        auto btntext = "Set recommended key count ("
            + std::to_string(recommended) + ")";
        if (ImGui::Button(btntext.c_str())) {
            G.key_count = recommended;
            key_buffer_regenerate = true;
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
        bool hb = ImGui::DragFloat3("Box high bound", glm::value_ptr(G.high_bound), 0.02, 0, 60);
        bool lb = ImGui::DragFloat3("Box low bound", glm::value_ptr(G.low_bound), 0.02, -60, 0);
        ImGui::SeparatorText("SPH settings");
        bool sr = ImGui::DragFloat("Smoothing radius", &G.smoothing_radius, 0.001, 0.01, 10);
        if (hb || lb || sr) {
            G.low_bound_cell = ivec3(floor(G.low_bound / G.smoothing_radius));
            G.high_bound_cell = ivec3(floor(G.high_bound / G.smoothing_radius));
            G.grid_size = G.high_bound_cell - G.low_bound_cell + ivec3(1, 1, 1);
        }
        ImGui::DragFloat("Target density", &G.target_density, 0.01, 0.01, 100);
        ImGui::DragFloat("Pressure multiplier", &G.pressure_multiplier, 0.1, 0.01, 1000);
        ImGui::DragFloat("Collision multiplier", &G.collision_multiplier, 0.01, 0.1, 1);
        ImGui::DragFloat("sigma", &G.sigma_viscosity, 0.05, 0, 1);
        ImGui::DragFloat("Near density multiplier", &G.near_density_multiplier, 0.5, 0, 10);
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
        globals_ssbo.set_data(sizeof(G), &G);
        globals_ssbo.bind();

        if (object_buffer_regenerate) {
            particles->output->resize(G.object_count * sizeof(Particle));
            if (prev_object_count < G.object_count) {
                generate_particles.uniform("prev_object_count", prev_object_count);
                generate_particles.dispatch_executions(
                    G.object_count
                );
            }
            prev_object_count = G.object_count;
            particles->output->resize(G.object_count * sizeof(Particle));
            object_buffer_regenerate = false;
        }

        if (key_buffer_regenerate) {
            keys->input->resize((G.key_count + 1) * sizeof(uint));
            keys->output->resize((G.key_count + 1) * sizeof(uint));
            key_buffer_regenerate = false;
        }

        if (G.key_count > 0) {
            // zero fill input key counts
            clear_keys.dispatch_executions(G.key_count + 1);

            // count keys
            predict_and_hash.dispatch_executions(G.object_count);

            // calculate key indicies
            for (uint offset = 1; offset < G.key_count + 1; offset *= 2) {
                prefix_sum.uniform("offset", offset);
                prefix_sum.dispatch_executions(G.key_count + 1);
            }

            // bucket sort using key indicies
            bucket_sort.dispatch_executions(G.object_count);
        }

        if (!paused) {
            density.dispatch_executions(G.object_count);
            viscosity.dispatch_executions(G.object_count);
            pressure_force.dispatch_executions(G.object_count);
            update_position.dispatch_executions(G.object_count);
        }

        particle_shader.use();
        particle_shader.uniform("view", camera.view());
        particle_shader.uniform("projection", camera.projection());
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        particles->input->bind();
        glBindVertexArray(empty_vao);
        glDrawArraysInstanced(GL_POINTS, 0, 1, G.object_count);

        box_shader.use();
        box_shader.uniform("view_projection", camera.view_projection());
        box_ssbo.bind();
        glBindVertexArray(empty_vao);
        glDrawArraysInstanced(GL_LINES, 0, 2, 12);
    });

    Context::uninit();
    return 0;
}
