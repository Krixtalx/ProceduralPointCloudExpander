#version 410

layout (location = 0) in vec3 vertice;
uniform mat4 matrizModVisProy;

void main() {
    gl_Position = matrizModVisProy * vec4 (vertice, 1);
}
