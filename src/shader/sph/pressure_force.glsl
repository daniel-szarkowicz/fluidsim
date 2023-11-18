// uses Particle from particle.glsl

layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 3) readonly buffer inputs {
    Particle p[];
};

layout(std430, binding = 4) writeonly buffer outputs {
    Particle po[];
};

float density_to_pressure(float density) {
    return (density - G.target_density) * G.pressure_multiplier;
}

void main() {
    uint i = gl_GlobalInvocationID.x;
    if (i >= G.object_count) {
        return;
    }
    vec4 pressure_force = vec4(0, 0, 0, 0);
    for (uint j = 0; j < G.object_count; ++j) {
        if (i != j) {
            float distance = distance(p[i].position, p[j].position);
            vec4 dir = (p[i].position - p[j].position)/distance;
            if (distance == 0.0) {
                // pseudo random
                float alpha = float(i)/float(G.object_count);
                dir = vec4(cos(alpha), sin(alpha), 0, 0);
            }
            float pressure = density_to_pressure(p[j].density);
            pressure_force -= pressure * dir * kernel_derived(distance)
                * p[j].mass / p[j].density;
        }
    }
    Particle particle = p[i];
    particle.velocity = pressure_force / particle.density * G.delta_time;
    po[i] = particle;
}
