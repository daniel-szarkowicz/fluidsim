#version 430

uniform mat4 view;

struct Sphere {
    vec4 center;
    vec4 color;
    float radius;
};

layout(std430, binding = 3) readonly buffer points {
    Sphere spheres[];
};

out vec3 vColor;
out float vRadius;

void main() {
    gl_Position = view * spheres[gl_InstanceID].center;
    vColor = spheres[gl_InstanceID].color.xyz;
    vRadius = spheres[gl_InstanceID].radius;
}
