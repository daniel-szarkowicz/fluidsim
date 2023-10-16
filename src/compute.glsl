#version 430

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

struct Sphere {
    vec4 center;
    vec4 color;
    float radius;
};

layout(std430, binding = 3) buffer points {
    Sphere spheres[];
};

void main() {
    spheres[gl_GlobalInvocationID.x].center += vec4(0.001, 0.001, 0.001, 0);
}
