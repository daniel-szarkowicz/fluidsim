// uses Sphere from sphere_struct.glsl

layout(std430, binding = 3) readonly buffer points {
    Sphere spheres[];
};

layout(std430, binding = 5) readonly buffer keys {
    uint map[];
};

uniform mat4 view;

out vec3 vColor;
out float vRadius;

uint hash(uint x) {
    x ^= x >> 17;
    x *= 0xed5ad4bbU;
    x ^= x >> 11;
    x *= 0xac4c1b51U;
    x ^= x >> 15;
    x *= 0x31848babU;
    x ^= x >> 14;
    return x;
}

uint cell_hash(vec4 position) {
    vec4 cell_pos = floor(position / 1.0);
    uint cell_hash = hash(floatBitsToUint(cell_pos.x));
    cell_hash = hash(cell_hash + floatBitsToUint(cell_pos.y));
    cell_hash = hash(cell_hash + floatBitsToUint(cell_pos.z));
    return cell_hash;
}

void main() {
    gl_Position = view * spheres[gl_InstanceID].center;
    // uint hash = spheres[gl_InstanceID].cell_hash;
    // vColor.x = float(hash & 7) / 7.0;
    // vColor.y = float((hash & (3 << 3)) >> 3) / 3.0;
    // vColor.z = float((hash & (7 << 5)) >> 5) / 7.0;
    uint key = spheres[gl_InstanceID].cell_key;
    uint chash = spheres[gl_InstanceID].cell_hash; 
    vColor.x = float(key & 7) / 7.0;
    vColor.y = float((key & (3 << 3)) >> 3) / 3.0;
    vColor.z = float((key & (7 << 5)) >> 5) / 7.0;
    // vColor = spheres[gl_InstanceID].color.xyz;
    // vColor = vec3(1, 1, 1) * (float(gl_InstanceID) / 10000);
    // vColor.x = 1;
    // gl_Position.x = float(gl_InstanceID % 100) - 50;
    // gl_Position.y = 0;
    // gl_Position.z = float(gl_InstanceID / 100) - 50;
    // gl_Position.w = 1;
    // gl_Position = view * (gl_Position/2);
    // if (gl_InstanceID < map[127] || gl_InstanceID >= map[128]) {
    //     gl_Position = view * vec4(0, 0, 0, 1);
    // }

    // vColor = vec3(0, 0, 0);
    // for (int xoff = -1; xoff <= 1; xoff += 1) {
    //     for (int yoff = -1; yoff <= 1; yoff += 1) {
    //         for (int zoff = -1; zoff <= 1; zoff += 1) {
    //             uint hash = cell_hash(vec4(xoff, yoff, zoff, 1));
    //             if (key == hash % 256) {
    //                 vColor += vec3(0.1, 0.1, 0.1);
    //             }
    //         }
    //     }
    // }
    
    vRadius = spheres[gl_InstanceID].radius;
}
