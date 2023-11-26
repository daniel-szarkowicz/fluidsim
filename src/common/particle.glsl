struct Particle {
    vec4 position;

    vec4 predicted_position;

    vec4 velocity;

    ivec4 cell_pos;

    float mass;
    float density;
    uint cell_key;
    uint id;

    uint index_in_key;
    float pressure;
    float _padding1;
    float _padding2;
};
