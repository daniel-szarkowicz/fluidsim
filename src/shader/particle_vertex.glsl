// uses particle from particle.glsl

layout(std430, binding = 3) readonly buffer points {
    Particle p[];
};

uniform mat4 view;

out vec4 vColor;
out float vRadius;

void main() {
    uint i = gl_InstanceID;
    gl_Position = view * p[i].position;
    // float relative_density = p[i].density - G.target_density;
    // if (relative_density < 0) {
    //     vColor = mix(vec4(1, 1, 1, 1), vec4(0, 1, 0, 1), log(1-relative_density));
    // } else {
    //     vColor = mix(vec4(1, 1, 1, 1), vec4(1, 0, 0, 1), log(1+relative_density));
    // }


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
        ivec4 s_cell = cell_pos(s.position);
        bool same_key = false;
        for (uint coi = 0; coi < 27; ++coi) {
            uint key = cell_key(cell_hash(s_cell + cell_neighbors[coi]));
            if (key == p[i].cell_key) {
                same_key = true;
            }
        }
        if (same_key) {
            vColor.r = 1;
        } else {
            vColor.r = float(p[i].cell_key) / 256;
            vColor.g = float(p[i].cell_key) / 256;
            vColor.b = float(p[i].cell_key) / 256;
        }
    }
    vRadius = G.particle_size;
}
