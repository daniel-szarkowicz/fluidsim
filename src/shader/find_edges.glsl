// uses Sphere from sphere_struct.glsl

layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 3) readonly buffer points {
    Sphere spheres[];
};

layout(std430, binding = 5) writeonly buffer keys {
    uint map[];
};

uniform uint object_count;

void main() {
    uint i = gl_GlobalInvocationID.x + 1;
    if (i > object_count) {
        return;
    }
    if (i == 0) {
        for (uint key = 0; key <= spheres[i].cell_key; ++key) {
            map[key] = i;
        }
    } else if (i == object_count) {
        for (uint key = spheres[i-1].cell_key + 1; key <= 256; ++key) {
            map[key] = i;
        }
    } else if (spheres[i-1].cell_key != spheres[i].cell_key) {
        for (uint key = spheres[i-1].cell_key + 1; key <= spheres[i].cell_key;
        ++key) {
            map[key] = i;
        }
    }
}
