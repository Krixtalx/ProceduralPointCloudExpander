#include "stdafx.h"
#include "ComputeShader.h"

std::vector<GLint> PPCX::ComputeShader::MAX_WORK_GROUP_SIZE = { 1024, 1024, 64 };	

void PPCX::ComputeShader::bindBuffers(const std::vector<GLuint>& bufferID) {
	for (unsigned i = 0; i < bufferID.size(); ++i) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, i, bufferID[i]);
	}
}

void PPCX::ComputeShader::execute(GLuint numGroups_x, GLuint numGroups_y, GLuint numGroups_z, GLuint workGroup_x,
								  GLuint workGroup_y, GLuint workGroup_z) {
	glDispatchComputeGroupSizeARB(numGroups_x, numGroups_y, numGroups_z, workGroup_x, workGroup_y, workGroup_z);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
}



std::vector<GLint> PPCX::ComputeShader::getMaxLocalSize() {
	std::vector<GLint> maxLocalSize(3);

	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &maxLocalSize[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &maxLocalSize[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &maxLocalSize[2]);

	return maxLocalSize;
}

int PPCX::ComputeShader::getNumGroups(const unsigned arraySize, const WorkGroupAxis axis) {
	return (int)std::ceil((float)arraySize / MAX_WORK_GROUP_SIZE[axis]);
}

int PPCX::ComputeShader::getWorkGroupSize(const unsigned numGroups, const unsigned arraySize) {
	return (int)std::ceil((float)arraySize / numGroups);
}

void PPCX::ComputeShader::initializeMaxGroupSize() {
	ComputeShader::MAX_WORK_GROUP_SIZE = getMaxLocalSize();
}
