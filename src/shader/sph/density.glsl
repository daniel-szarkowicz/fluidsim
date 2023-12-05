void main() {
    uint i = gl_GlobalInvocationID.x;
    if (i >= G.object_count) {
        return;
    }
    float density = 0;
    float near_density = 0;
    for_neighbor(p[i], neighbor, {
        float distance = distance(p[i].predicted_position, neighbor.predicted_position);
        density += neighbor.mass * kernel(distance);
        float near_value = viscosity_kernel(distance);
        near_density += near_value * near_value * near_value ; 
    });
    Particle particle = p[i];
    particle.density = density;
    particle.pressure = (density - G.target_density) * G.pressure_multiplier;
    particle.near_density = near_density;
    po[i] = particle;
}
