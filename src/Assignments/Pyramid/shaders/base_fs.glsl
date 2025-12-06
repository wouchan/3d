#version 420

layout(location=0) in vec3 vertexColor;

layout(location=0) out vec4 vFragColor;

layout(std140, binding = 0) uniform Modifier {
    float strength;
    vec3  color; 
};

void main() {
    vFragColor = vec4(vertexColor * color * strength, 1.0);
}
