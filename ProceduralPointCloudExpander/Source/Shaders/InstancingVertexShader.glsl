#version 410

layout (location = 0) in vec3 vPosition;
layout (location = 1) in uint vColor;
layout (location = 2) in mat4 offset;

uniform mat4 matrizVP;	

out vec4 ourColor;

void main() {
	gl_Position = matrizVP * offset * vec4(vPosition, 1.0);
	ourColor = unpackUnorm4x8(vColor);
	ourColor[3]=1;
}