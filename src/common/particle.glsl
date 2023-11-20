struct Particle {
    vec4 position;

    vec4 predicted_position;

    vec4 velocity;

    float mass;
    float density;
    uint cell_hash;
    uint cell_key;
};
