#define for_neighbor(particle, neighbor, block)                                \
for (uint _i_ = 0; _i_ < G.object_count; ++_i_) {                              \
    Particle neighbor = p[_i_];                                                \
    if (distance(particle.position, neighbor.position) <= G.smoothing_radius) {\
        block                                                                  \
    }                                                                          \
}

