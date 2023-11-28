void main() {
    uint i = gl_GlobalInvocationID.x;
    if (i >= G.object_count) {
        return;
    }
    Particle particle = p[i];
    particle.velocity += G.gravity * G.delta_time;
    particle.position += particle.velocity * G.delta_time;
    if (particle.position.x > G.high_bound.x) {
        particle.position.x = G.high_bound.x;
        particle.velocity.x = -abs(particle.velocity.x) * G.collision_multiplier;
    }
    if (particle.position.y > G.high_bound.y) {
        particle.position.y = G.high_bound.y;
        particle.velocity.y = -abs(particle.velocity.y) * G.collision_multiplier;
    }
    if (particle.position.z > G.high_bound.z) {
        particle.position.z = G.high_bound.z;
        particle.velocity.z = -abs(particle.velocity.z) * G.collision_multiplier;
    }
    if (particle.position.x < G.low_bound.x) {
        particle.position.x = G.low_bound.x;
        particle.velocity.x = abs(particle.velocity.x) * G.collision_multiplier;
    }
    if (particle.position.y < G.low_bound.y) {
        particle.position.y = G.low_bound.y;
        particle.velocity.y = abs(particle.velocity.y) * G.collision_multiplier;
    }
    if (particle.position.z < G.low_bound.z) {
        particle.position.z = G.low_bound.z;
        particle.velocity.z = abs(particle.velocity.z) * G.collision_multiplier;
    }
    po[i] = particle;
}
