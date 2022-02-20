#version 410

layout (location = 0) in vec3 vPosition;
layout (location = 1) in uint vColor;
layout (location = 2) in vec3 offset;

uniform mat4 matrizMVP;	

out vec4 ourColor;

void main() {
	gl_Position = matrizMVP * vec4(vPosition + offset, 1.0);
	ourColor = unpackUnorm4x8(vColor);
	ourColor[3]=1;
}