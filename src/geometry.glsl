#version 400
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform float radius;

in vec3 vColor[];
out vec3 fColor;
out vec3 fCenter;
out vec3 fPosition;

void main() {
    fColor = vColor[0];
    fCenter = gl_in[0].gl_Position.xyz;

    gl_Position = gl_in[0].gl_Position + vec4(-radius, -radius, 0.0, 0.0);
    fPosition = gl_Position.xyz;
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4(-radius, radius, 0.0, 0.0);
    fPosition = gl_Position.xyz;
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4(radius, -radius, 0.0, 0.0);
    fPosition = gl_Position.xyz;
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4(radius, radius, 0.0, 0.0);
    fPosition = gl_Position.xyz;
    EmitVertex();
    EndPrimitive();
}
