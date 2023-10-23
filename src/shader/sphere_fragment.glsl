in vec3 fColor;
in vec2 fPosition;

out vec4 frag_color;

void main() {
    if (length(fPosition) < 1.0) {
        frag_color = vec4(fColor, 1.0);
    } else {
        discard;
    }
}
