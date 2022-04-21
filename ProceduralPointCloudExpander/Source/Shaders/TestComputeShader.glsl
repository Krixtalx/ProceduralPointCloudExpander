#version 450
#extension GL_ARB_compute_variable_group_size: enable
#extension GL_NV_gpu_shader5: enable
layout(local_size_variable) in;

layout (std430, binding = 0) buffer Buffer		{ uint			testBuffer[]; };

void main() {
	uint index = gl_GlobalInvocationID.x;
	if (index >= 100) return;
	testBuffer[index] = index;
}