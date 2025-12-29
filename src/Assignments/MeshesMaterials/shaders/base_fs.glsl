#version 420

layout(location=0) out vec4 vFragColor;

layout(std140, binding = 0) uniform Color {
    vec4 color;
};

void main() {
    vFragColor = color;
}
