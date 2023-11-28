void main() {
    uint i = gl_GlobalInvocationID.x;
    if (i >= G.object_count) {
        return;
    }

    Particle particle = p[i];
    po[key_map[particle.cell_key] + particle.index_in_key] = particle;
}
