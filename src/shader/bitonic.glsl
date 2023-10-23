// uses Sphere from sphere_struct.glsl

layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 3) buffer inputs {
    Sphere spheres[];
};

uniform uint object_count;

void compare_and_swap(uint first, uint second) {
    if (first < object_count && second < object_count) {
        // TODO: use cell key instead
        if(spheres[first].radius > spheres[second].radius) {
            Sphere tmp = spheres[first];
            spheres[first] = spheres[second];
            spheres[second] = tmp;
        }
    }
    memoryBarrier();
    barrier();
}

void bitonic_sort() {
    uint i = gl_GlobalInvocationID.x;
    uint n = 1;
    while (n < object_count) {
        n *= 2;
    }
    for (uint block_size = 2; block_size <= n; block_size *= 2) {
        uint block_start = (i / (block_size / 2)) * block_size;
        uint offset_in_block = i % (block_size / 2);
        compare_and_swap(block_start + offset_in_block,
                         block_start + block_size - 1 - offset_in_block);
        for (uint sub_block_size = block_size / 2;
             sub_block_size > 1; sub_block_size /= 2
        ) {
            uint sub_block_start_offset =
                (offset_in_block / (sub_block_size / 2)) * sub_block_size;
            uint offset_in_sub_block = offset_in_block % (sub_block_size / 2);
            uint sub_block_start = block_start + sub_block_start_offset;
            compare_and_swap(sub_block_start + offset_in_sub_block,
                sub_block_start + offset_in_sub_block + sub_block_size / 2);
        }
    }
}

void main() {
    bitonic_sort();
}
