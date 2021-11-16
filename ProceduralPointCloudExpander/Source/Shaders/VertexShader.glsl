#version 450

layout (location = 0) in vec3 vPosition;
layout (location = 1) in uint vColor;

uniform mat4 matrizMVP;	

out vec4 ourColor;

void main() {
	gl_PointSize = 10;
	gl_Position = matrizMVP * vec4(vPosition, 1.0);
	ourColor = unpackUnorm4x8(vColor);
}