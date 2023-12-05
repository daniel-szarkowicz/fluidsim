void main() {
    uint i = gl_GlobalInvocationID.x;
    if (i >= G.object_count) {
        return;
    }
    Particle particle = p[i];
    particle.predicted_position =
        particle.position + particle.velocity * G.delta_time;
    particle.cell_pos = cell_pos(particle.predicted_position);
    particle.cell_key = cell_key(particle.cell_pos);
    particle.index_in_key = atomicAdd(k[particle.cell_key + 1], 1);
    po[i] = particle;
}
