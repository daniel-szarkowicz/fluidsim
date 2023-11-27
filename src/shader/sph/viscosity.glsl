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

void main(){
    uint i = gl_GlobalInvocationID.x;
    if (i >= G.object_count) {
        return;
    }
    vec4 viscosity_impulse = vec4(0, 0, 0, 0);

    for_neighbor(p[i], neighbor, {
        if (p[i].id != neighbor.id) {
            float visc_dst = distance(p[i].position, neighbor.position);
            if(visc_dst > 0.0){     
                vec4 visc_dir = (neighbor.position - p[i].position)/visc_dst;
                vec4 u = (neighbor.velocity - p[i].velocity) * visc_dir;
                if(length(u) > 0.0){
                    viscosity_impulse += viscosity_kernel(visc_dst) * (G.sigma_viscosity * u) * visc_dir *1;
                }
            }
        }
    });
    Particle particle = p[i];
    particle.velocity += viscosity_impulse * G.delta_time;
    po[i] = particle;
}