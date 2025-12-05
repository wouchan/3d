#version 420

layout(location=0) in  vec3 a_vertex_position;
layout(location=1) in  vec3 a_vertex_color;

layout(location=0) out vec3 vertexColor;

layout(std140, binding=1) uniform Transformations {
    vec2 scale;
    vec2 translation;
    mat2 rotation;
};

void main() {
    gl_Position.xy = rotation * (scale * a_vertex_position.xy) + translation;
    gl_Position.zw = vec2(a_vertex_position.z, 1.0);
    vertexColor = a_vertex_color;
}
