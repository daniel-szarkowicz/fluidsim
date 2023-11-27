// uses particle from particle.glsl

layout(std430, binding = 3) readonly buffer points {
    Particle p[];
};

layout(std430, binding = 2) readonly buffer keys {
    uint key_map[];
};

uniform mat4 view;

out vec4 vColor;
out float vRadius;

void main() {
    uint i = gl_InstanceID;
    gl_Position = view * vec4(p[i].position, 1);
    vRadius = G.particle_size;
    vColor = vec4(1, 0, 1, 1);
    if (G.key_count == 0) {
        return;
    }

    switch(G.visualization) {
        case VISUALIZATION_DENSITY: {
            float relative_density = p[i].density / G.target_density - 1;
            relative_density *= G.density_color_multiplier;
            if (relative_density < 0) {
                vColor = mix(
                    vec4(1, 1, 1, 1),
                    vec4(0, 1, 0, 1),
                    log(1-relative_density)
                );
            } else {
                vColor = mix(
                    vec4(1, 1, 1, 1),
                    vec4(1, 0, 0, 1),
                    log(1+relative_density)
                );
            }
        } break;
        case VISUALIZATION_CELL_KEY_EXPECTED: {
            if (G.selected_index >= G.object_count) {
                return;
            }
            if(G.selected_index == i) {
                vColor = vec4(0, 0, 1, 1);
            } else {
                Particle s = p[G.selected_index];
                vColor = vec4(0, 0, 0, 1);
                if (
                    distance(p[i].position, s.position)
                    < G.smoothing_radius
                ) {
                    vColor.g = 1;
                }
                ivec3 s_cell = s.cell_pos;
                bool same_key = false;
                for (uint coi = 0; coi < 27; ++coi) {
                    uint key = cell_key(s_cell + cell_neighbors[coi]);
                    if (key == p[i].cell_key) {
                        same_key = true;
                    }
                }
                if (same_key) {
                    vColor.r = 1;
                } else {
                    vColor.r = float(p[i].cell_key) / G.key_count;
                    vColor.g = float(p[i].cell_key) / G.key_count;
                    vColor.b = float(p[i].cell_key) / G.key_count;
                }
            }
        } break;
        case VISUALIZATION_CELL_KEY_ACTUAL: {
            if (G.selected_index >= G.object_count) {
                return;
            }
            if(G.selected_index == i) {
                vColor = vec4(0, 0, 1, 1);
            } else {
                Particle s = p[G.selected_index];
                vColor = vec4(0, 0, 0, 1);
                bool colored = false;
                for_neighbor_unchecked(s, neighbor, {
                    if (neighbor.id == p[i].id) {
                        vColor.r = 1;
                        colored = true;
                    }
                });
                for_neighbor(s, neighbor, {
                    if (neighbor.id == p[i].id) {
                        vColor.g = 1;
                        colored = true;
                    }
                });
                if (!colored) {
                    vColor = vec4(1, 0, 1, 1);
                }
            }
        } break;
        case VISUALIZATION_SPEED: {
            float speed = length(p[i].velocity)/3;
            vColor = mix(
                vec4(0, 0, 1, 1),
                vec4(1, 0, 0, 1),
                speed
            );
        }
    }
}
