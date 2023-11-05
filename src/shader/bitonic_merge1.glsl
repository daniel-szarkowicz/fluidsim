// uses Sphere from sphere_struct.glsl

layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 3) buffer inputs {
    Sphere spheres[];
};

uniform uint object_count;
uniform uint block;

void compare_and_swap(uint first, uint second) {
    if (first < object_count && second < object_count) {
        if(spheres[first].cell_key > spheres[second].cell_key) {
            Sphere tmp = spheres[first];
            spheres[first] = spheres[second];
            spheres[second] = tmp;
        }
    }
}

void main() {
    uint i = gl_GlobalInvocationID.x;
    uint start = (i / (block / 2)) * block;
    uint offset = i % (block / 2);
    compare_and_swap(start + block / 2 - 1 - offset,
                     start + block / 2 + offset);
}
