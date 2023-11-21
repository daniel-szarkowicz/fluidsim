struct Particle {
    vec4 position;

    vec4 predicted_position;

    vec4 velocity;

    float mass;
    float density;
    uint cell_hash;
    uint cell_key;

    uint id;
    float _padding1;
    float _padding2;
    float _padding3;
};
