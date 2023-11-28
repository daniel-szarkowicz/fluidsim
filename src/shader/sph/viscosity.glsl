void main(){
    uint i = gl_GlobalInvocationID.x;
    if (i >= G.object_count) {
        return;
    }
    vec3 viscosity_impulse = vec3(0, 0, 0);

    for_neighbor(p[i], neighbor, {
        if (p[i].id != neighbor.id) {
            float visc_dst = distance(p[i].position, neighbor.position);
            if(visc_dst > 0.0){     
                vec3 visc_dir = (neighbor.position - p[i].position)/visc_dst;
                vec3 u = (neighbor.velocity - p[i].velocity) * visc_dir;
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
