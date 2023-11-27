struct Globals {
    vec3 gravity;
    uint object_count;

    vec3 low_bound;
    float particle_size;

    vec3 high_bound;
    float smoothing_radius;

    ivec3 low_bound_cell;
    float delta_time;

    ivec3 high_bound_cell;
    float target_density;

    ivec3 grid_size;
    float pressure_multiplier;

    float collision_multiplier;
    uint key_count;
    uint selected_index;
    uint visualization;

    float density_color_multiplier;
    float sigma_viscosity;
    float near_density_multiplier;
    float _padding1;
};

#define VISUALIZATION_DENSITY 0
#define VISUALIZATION_CELL_KEY_EXPECTED 1
#define VISUALIZATION_CELL_KEY_ACTUAL 2
#define VISUALIZATION_SPEED 3
