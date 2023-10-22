#version 430


struct Sphere {
    vec4 center;
    vec4 velocity;
    vec4 color;
	float density;
	float mass;
	float pressure;
    float radius;
};

layout(std430, binding = 3) readonly buffer points {
    Sphere spheres[];
};

uniform mat4 view;

out vec3 vColor;
out float vRadius;

void main() {
    gl_Position = view * spheres[gl_InstanceID].center;
    vColor = spheres[gl_InstanceID].color.xyz;
    vRadius = spheres[gl_InstanceID].radius;
}
