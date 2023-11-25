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

float density_to_pressure(float density) {
    return (density - G.target_density) * G.pressure_multiplier;
}

float density_to_near_pressure(float near_density){
    return 2 * near_density;
}

void main() {
    uint i = gl_GlobalInvocationID.x;
    if (i >= G.object_count) {
        return;
    }
    vec4 pressure_force = vec4(0, 0, 0, 0);
    vec4 viscosity_impulse = vec4(0, 0, 0, 0);
    float sigma = 0.4;
    float beta = 0;
    for_neighbor(p[i], neighbor, {
        if (p[i].id != neighbor.id) {
            float visc_dst = distance(p[i].position, neighbor.position);
            float dst = distance(p[i].predicted_position, neighbor.predicted_position);
            vec4 dir = (p[i].predicted_position - neighbor.predicted_position)/dst;
            if (dst == 0.0) {
                // pseudo random
                float alpha = float(i)/float(G.object_count);
                dir = vec4(cos(alpha), sin(alpha), 0, 0);
            }
            if(visc_dst > 0.0){     
                vec4 visc_dir = (neighbor.position - p[i].position)/visc_dst;
                vec4 u = (neighbor.velocity - p[i].velocity) * visc_dir;
                if(length(u) > 0.0){
                    //viscosity_impulse += (neighbor.velocity - p[i].velocity) * viscosity_kernel(visc_dst);
                    viscosity_impulse += viscosity_kernel(visc_dst) * (sigma * u + beta * u * u) * visc_dir *1;
                }
            }
            float pressure = (density_to_pressure(p[i].density)
                + density_to_pressure(neighbor.density))/2;
            pressure_force -= pressure * dir * kernel_derived(dst)
                * neighbor.mass / neighbor.density;

            float near_density_kernel_derived_value = 3 * viscosity_kernel(dst) * viscosity_kernel(dst);
            pressure_force -= dir *  near_density_kernel_derived_value * 0.5 * (density_to_near_pressure(p[i].near_density) + density_to_near_pressure(neighbor.near_density)) / neighbor.near_density;
        }
    });
    Particle particle = p[i];
    particle.velocity += viscosity_impulse * G.delta_time;
    particle.velocity += pressure_force / particle.density * G.delta_time;
    po[i] = particle;
}
