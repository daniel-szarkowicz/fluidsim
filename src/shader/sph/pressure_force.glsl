// uses Particle from particle.glsl

layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 3) readonly buffer inputs {
    Particle p[];
};

layout(std430, binding = 4) writeonly buffer outputs {
    Particle po[];
};

layout(std430, binding = 2) readonly buffer keys {
    uint key_map[];
};

float density_to_near_pressure(float near_density){
    return G.near_density_multiplier * near_density;
}

void main() {
    uint i = gl_GlobalInvocationID.x;
    if (i >= G.object_count) {
        return;
    }
    vec3 pressure_force = vec3(0, 0, 0);
    for_neighbor(p[i], neighbor, {
        if (p[i].id != neighbor.id) {
            float distance = distance(p[i].predicted_position, neighbor.predicted_position);
            vec3 dir = (p[i].predicted_position - neighbor.predicted_position)/distance;
            if (distance == 0.0) {
                // pseudo random
                float alpha = float(i)/float(G.object_count);
                dir = vec3(cos(alpha), sin(alpha), 0);
            }
            float pressure = (p[i].pressure + neighbor.pressure)/2;
            pressure_force -= pressure * dir * kernel_derived(distance)
                * neighbor.mass / neighbor.density;

            float near_density_kernel_derived_value = -3/G.smoothing_radius * viscosity_kernel(distance) * viscosity_kernel(distance);       
            pressure_force -= dir *  near_density_kernel_derived_value * 0.5 * (density_to_near_pressure(p[i].near_density) + density_to_near_pressure(neighbor.near_density)) / neighbor.near_density;
        }
    });
    Particle particle = p[i];
    particle.velocity += pressure_force / particle.density * G.delta_time;
    po[i] = particle;
}
