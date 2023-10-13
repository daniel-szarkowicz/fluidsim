#version 400

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in vec3 vColor[];
in float vRadius[];

out vec3 fCenter;
out vec3 fColor;
out float fRadius;
out vec3 fPosition;

void main() {
    fCenter = gl_in[0].gl_Position.xyz;
    fColor = vColor[0];
    fRadius = vRadius[0];

    gl_Position = gl_in[0].gl_Position + vec4(-fRadius, -fRadius, 0.0, 0.0);
    fPosition = gl_Position.xyz;
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4(-fRadius, fRadius, 0.0, 0.0);
    fPosition = gl_Position.xyz;
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4(fRadius, -fRadius, 0.0, 0.0);
    fPosition = gl_Position.xyz;
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4(fRadius, fRadius, 0.0, 0.0);
    fPosition = gl_Position.xyz;
    EmitVertex();
    EndPrimitive();
}
