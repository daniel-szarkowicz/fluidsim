in vec4 fColor;
in vec4 fPosition;

out vec4 frag_color;

void main() {
    if (length(fPosition) < 1.0) {
        frag_color = fColor;
    } else {
        discard;
    }
}
