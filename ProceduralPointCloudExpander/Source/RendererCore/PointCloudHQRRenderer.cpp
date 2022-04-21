#include "stdafx.h"
#include "PointCloudHQRRenderer.h"

#include "Renderer.h"
#include "ShaderManager.h"

PointCloudHQRRenderer::~PointCloudHQRRenderer() {
	glDeleteBuffers(1, &_color01SSBO);
	glDeleteBuffers(1, &_color02SSBO);
	glDeleteBuffers(1, &_depthBufferSSBO);
	glDeleteBuffers(1, &_rawDepthBufferSSBO);
	glDeleteTextures(1, &_textureID);
}

void PointCloudHQRRenderer::addPointCloud(const std::string& name, PointCloud* pointCloud) {
	auto previous = pointCloudVBOs.find(name);
	if (previous != pointCloudVBOs.end()) {
		glDeleteBuffers(1, &previous->second);
	}
	GLuint id = PPCX::ComputeShader::setReadBuffer(pointCloud->getPoints().data(), pointCloud->getNumberOfPoints());
	pointCloudVBOs.insert_or_assign(name, id);
	pointCloudNpoints.insert_or_assign(name, pointCloud->getNumberOfPoints());
}

void PointCloudHQRRenderer::render(const mat4& MVPmatrix) {
	const int numGroupsImage = PPCX::ComputeShader::getNumGroups(windowSize.x * windowSize.y);

	// 1. Fill buffer of 32 bits with UINT_MAX
	_resetDepthBufferHQRShader->bindBuffers(std::vector<GLuint> { _rawDepthBufferSSBO, _color01SSBO, _color02SSBO });
	PPCX::ShaderManager::getInstancia()->activarSP("ResetComputeShaderSP");
	PPCX::ShaderManager::getInstancia()->setUniform("ResetComputeShaderSP", "windowSize", windowSize);
	_resetDepthBufferHQRShader->execute(numGroupsImage, 1, 1, PPCX::ComputeShader::getMaxGroupSize(), 1, 1);

	for (auto pointCloud : pointCloudVBOs) {
		const unsigned numPoints = pointCloudNpoints.find(pointCloud.first)->second;
		const int numGroupsPoints = PPCX::ComputeShader::getNumGroups(numPoints);

		// 2. Transform points and use atomicMin to retrieve the nearest point
		_DepthHQRShader->bindBuffers(std::vector{ _rawDepthBufferSSBO, pointCloud.second });
		PPCX::ShaderManager::getInstancia()->activarSP("DepthComputeShaderSP");
		PPCX::ShaderManager::getInstancia()->setUniform("DepthComputeShaderSP", "windowSize", windowSize);
		PPCX::ShaderManager::getInstancia()->setUniform("DepthComputeShaderSP", "numPoints", numPoints);
		PPCX::ShaderManager::getInstancia()->setUniform("DepthComputeShaderSP", "cameraMatrix", MVPmatrix);
		_DepthHQRShader->execute(numGroupsPoints, 1, 1, PPCX::ComputeShader::getMaxGroupSize(), 1, 1);

		// 3. Accumulate colors once the minimum depth is defined
		_addColorsHQRShader->bindBuffers(std::vector<GLuint> { _rawDepthBufferSSBO, _color01SSBO, _color02SSBO, pointCloud.second });
		PPCX::ShaderManager::getInstancia()->activarSP("ColorComputeShaderSP");
		PPCX::ShaderManager::getInstancia()->setUniform("ColorComputeShaderSP", "windowSize", windowSize);
		PPCX::ShaderManager::getInstancia()->setUniform("ColorComputeShaderSP", "numPoints", numPoints);
		PPCX::ShaderManager::getInstancia()->setUniform("ColorComputeShaderSP", "cameraMatrix", MVPmatrix);
		PPCX::ShaderManager::getInstancia()->setUniform("ColorComputeShaderSP", "distanceThreshold", 1);
		_addColorsHQRShader->execute(numGroupsPoints, 1, 1, PPCX::ComputeShader::getMaxGroupSize(), 1, 1);
	}

	_storeHQRTexture->bindBuffers(std::vector<GLuint> { _color01SSBO, _color02SSBO });
	PPCX::ShaderManager::getInstancia()->activarSP("StoreComputeShaderSP");
	glBindImageTexture(0, _textureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
	PPCX::ShaderManager::getInstancia()->setUniform("StoreComputeShaderSP", "backgroundColor", PPCX::Renderer::getInstancia()->getColorFondo());
	PPCX::ShaderManager::getInstancia()->setUniform("StoreComputeShaderSP", "texImage", GLint(0));
	PPCX::ShaderManager::getInstancia()->setUniform("StoreComputeShaderSP", "windowSize", windowSize);
	_storeHQRTexture->execute(numGroupsImage, 1, 1, PPCX::ComputeShader::getMaxGroupSize(), 1, 1);

	__glewDrawTextureNV(_textureID, GL_TEXTURE0, 0, 0, windowSize.x, windowSize.y, 0, 0, 0, 1, 1);
}

void PointCloudHQRRenderer::updateWindowSize(const vec2 newWindowSize) {
	PPCX::ComputeShader::updateWriteBuffer(_depthBufferSSBO, uint64_t(), windowSize.x * windowSize.y, GL_DYNAMIC_DRAW);
	PPCX::ComputeShader::updateWriteBuffer(_rawDepthBufferSSBO, GLuint(), windowSize.x * windowSize.y, GL_DYNAMIC_DRAW);
	PPCX::ComputeShader::updateWriteBuffer(_color01SSBO, uint64_t(), windowSize.x * windowSize.y, GL_DYNAMIC_DRAW);
	PPCX::ComputeShader::updateWriteBuffer(_color02SSBO, uint64_t(), windowSize.x * windowSize.y, GL_DYNAMIC_DRAW);

	// Update size of texture
	glBindTexture(GL_TEXTURE_2D, _textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, windowSize.x, windowSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glGenerateMipmap(GL_TEXTURE_2D);
}
