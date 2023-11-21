struct Globals {
    vec4 gravity;

    vec3 low_bound;
    float _padding1;

    vec3 high_bound;
    float _padding2;

    uint object_count;
    float particle_size;
    float smoothing_radius;
    float delta_time;

    float target_density;
    float pressure_multiplier;
    float collision_multiplier;
    uint key_count;

    uint selected_index;
    uint visualization;
    float density_color_multiplier;
    float _padding3;
};

#define VISUALIZATION_DENSITY 0
#define VISUALIZATION_CELL_KEY 1
#define VISUALIZATION_KEY_INDEX 2
