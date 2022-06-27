#include "stdafx.h"
#include "ComputeShader.h"

#include <utility>

std::vector<GLint> PPCX::ComputeShader::MAX_WORK_GROUP_SIZE = { 1024, 1024, 64 };

PPCX::ComputeShader::ComputeShader(std::string nombreShader, const std::string& ruta) :
	Shader(std::move(nombreShader), GL_COMPUTE_SHADER, ruta) {}

void PPCX::ComputeShader::bindBuffers(const std::vector<GLuint>& bufferID) {
	for (unsigned i = 0; i < bufferID.size(); ++i) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, i, bufferID[i]);
	}
}

void PPCX::ComputeShader::execute(const GLuint numGroups_x, const GLuint numGroups_y, const GLuint numGroups_z, const GLuint workGroup_x,
                                  const GLuint workGroup_y, const GLuint workGroup_z) {
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
	return static_cast<int>(std::ceil(static_cast<float>(arraySize) / MAX_WORK_GROUP_SIZE[axis]));
}

int PPCX::ComputeShader::getWorkGroupSize(const unsigned numGroups, const unsigned arraySize) {
	return static_cast<int>(std::ceil(static_cast<float>(arraySize) / numGroups));
}

void PPCX::ComputeShader::initializeMaxGroupSize() {
	MAX_WORK_GROUP_SIZE = getMaxLocalSize();
}
