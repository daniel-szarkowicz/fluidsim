#version 430


struct Sphere {
    vec4 center;
    vec4 velocity;
    vec4 color;
    float radius;
    uint cell_hash;
    uint obj_id;
};

layout(std430, binding = 3) readonly buffer points {
    Sphere spheres[];
};

uniform mat4 view;

out vec3 vColor;
out float vRadius;

void main() {
    // gl_Position = view * spheres[gl_InstanceID].center;
    uint hash = spheres[gl_InstanceID].cell_hash;
    vColor.x = float(hash & 7) / 7.0;
    vColor.y = float((hash & (3 << 3)) >> 3) / 3.0;
    vColor.z = float((hash & (7 << 5)) >> 5) / 7.0;
    // vColor = spheres[gl_InstanceID].color.xyz;
    // vColor = vec3(1, 1, 1) * (float(gl_InstanceID) / 10000);
    // vColor.x = 1;
    gl_Position.x = float(gl_InstanceID % 100) - 50;
    gl_Position.y = 0;
    gl_Position.z = float(gl_InstanceID / 100) - 50;
    gl_Position.w = 1;
    gl_Position = view * (gl_Position/2);
    vRadius = spheres[gl_InstanceID].radius;
}
