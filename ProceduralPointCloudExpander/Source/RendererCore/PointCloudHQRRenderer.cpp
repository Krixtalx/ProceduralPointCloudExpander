#include "stdafx.h"
#include "PointCloudHQRRenderer.h"

#include "Renderer.h"
#include "ShaderManager.h"
#include "RenderOptions.h"

PointCloudHQRRenderer::PointCloudHQRRenderer() {
	resetDepthBufferHQRShader = PPCX::ShaderManager::getInstancia()->getComputeShader("resetBuffersHQR");
	depthHQRShader = PPCX::ShaderManager::getInstancia()->getComputeShader("depthBufferHQR");
	addColorsHQRShader = PPCX::ShaderManager::getInstancia()->getComputeShader("addColorHQR");
	storeHQRTexture = PPCX::ShaderManager::getInstancia()->getComputeShader("storeTextureHQR");

	windowSize = { PPCX::anchoVentanaPorDefecto, PPCX::altoVentanaPorDefecto };

	_color01SSBO = PPCX::ComputeShader::setWriteBuffer(uint64_t(), windowSize.x * windowSize.y, GL_DYNAMIC_DRAW);
	_color02SSBO = PPCX::ComputeShader::setWriteBuffer(uint64_t(), windowSize.x * windowSize.y, GL_DYNAMIC_DRAW);
	_depthBufferSSBO = PPCX::ComputeShader::setWriteBuffer(uint64_t(), windowSize.x * windowSize.y, GL_DYNAMIC_DRAW);
	_rawDepthBufferSSBO = PPCX::ComputeShader::setWriteBuffer(GLuint(), windowSize.x * windowSize.y, GL_DYNAMIC_DRAW);

	// Window texture
	glGenTextures(1, &_textureID);
	glBindTexture(GL_TEXTURE_2D, _textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, windowSize.x, windowSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glGenerateMipmap(GL_TEXTURE_2D);

	GLuint fboId = 0;
	glGenFramebuffers(1, &fboId);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fboId);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
						   GL_TEXTURE_2D, _textureID, 0);
}

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
	pointClouds.insert_or_assign(name, pointCloud);
}

void PointCloudHQRRenderer::render(const mat4& MVPmatrix) {
	const int numGroupsImage = PPCX::ComputeShader::getNumGroups(windowSize.x * windowSize.y);

	// 1. Fill buffer of 32 bits with UINT_MAX
	resetDepthBufferHQRShader->bindBuffers(std::vector<GLuint> { _rawDepthBufferSSBO, _color01SSBO, _color02SSBO });
	PPCX::ShaderManager::getInstancia()->activarSP("ResetComputeShaderSP");
	PPCX::ShaderManager::getInstancia()->setUniform("ResetComputeShaderSP", "windowSize", windowSize);
	resetDepthBufferHQRShader->execute(numGroupsImage, 1, 1, PPCX::ComputeShader::getMaxGroupSize(), 1, 1);

	for (const auto& pointCloud : pointCloudVBOs) {
		const auto cloud = pointClouds.find(pointCloud.first)->second;
		if (cloud->getVisibility()) {
			const unsigned numPoints = cloud->getNumberOfPoints();
			const int numGroupsPoints = PPCX::ComputeShader::getNumGroups(numPoints);

			// 2. Transform points and use atomicMin to retrieve the nearest point
			depthHQRShader->bindBuffers(std::vector{ _rawDepthBufferSSBO, pointCloud.second });
			PPCX::ShaderManager::getInstancia()->activarSP("DepthComputeShaderSP");
			PPCX::ShaderManager::getInstancia()->setUniform("DepthComputeShaderSP", "windowSize", windowSize);
			PPCX::ShaderManager::getInstancia()->setUniform("DepthComputeShaderSP", "numPoints", numPoints);
			PPCX::ShaderManager::getInstancia()->setUniform("DepthComputeShaderSP", "cameraMatrix", MVPmatrix);
			depthHQRShader->execute(numGroupsPoints, 1, 1, PPCX::ComputeShader::getMaxGroupSize(), 1, 1);

			// 3. Accumulate colors once the minimum depth is defined
			addColorsHQRShader->bindBuffers(std::vector<GLuint> { _rawDepthBufferSSBO, _color01SSBO, _color02SSBO, pointCloud.second });
			PPCX::ShaderManager::getInstancia()->activarSP("ColorComputeShaderSP");
			PPCX::ShaderManager::getInstancia()->setUniform("ColorComputeShaderSP", "windowSize", windowSize);
			PPCX::ShaderManager::getInstancia()->setUniform("ColorComputeShaderSP", "numPoints", numPoints);
			PPCX::ShaderManager::getInstancia()->setUniform("ColorComputeShaderSP", "cameraMatrix", MVPmatrix);
			PPCX::ShaderManager::getInstancia()->setUniform("ColorComputeShaderSP", "distanceThreshold", 1.01f);
			addColorsHQRShader->execute(numGroupsPoints, 1, 1, PPCX::ComputeShader::getMaxGroupSize(), 1, 1);
		}
	}

	storeHQRTexture->bindBuffers(std::vector<GLuint> { _color01SSBO, _color02SSBO });
	PPCX::ShaderManager::getInstancia()->activarSP("StoreComputeShaderSP");
	glBindImageTexture(0, _textureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
	PPCX::ShaderManager::getInstancia()->setUniform("StoreComputeShaderSP", "backgroundColor", PPCX::Renderer::getInstancia()->getColorFondo());
	PPCX::ShaderManager::getInstancia()->setUniform("StoreComputeShaderSP", "texImage", GLint(0));
	PPCX::ShaderManager::getInstancia()->setUniform("StoreComputeShaderSP", "windowSize", windowSize);
	storeHQRTexture->execute(numGroupsImage, 1, 1, PPCX::ComputeShader::getMaxGroupSize(), 1, 1);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, windowSize.x, windowSize.y, 0, 0, windowSize.x, windowSize.y,
					  GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void PointCloudHQRRenderer::updateWindowSize(const uvec2 newWindowSize) {
	PPCX::ComputeShader::updateWriteBuffer(_depthBufferSSBO, uint64_t(), windowSize.x * windowSize.y, GL_DYNAMIC_DRAW);
	PPCX::ComputeShader::updateWriteBuffer(_rawDepthBufferSSBO, GLuint(), windowSize.x * windowSize.y, GL_DYNAMIC_DRAW);
	PPCX::ComputeShader::updateWriteBuffer(_color01SSBO, uint64_t(), windowSize.x * windowSize.y, GL_DYNAMIC_DRAW);
	PPCX::ComputeShader::updateWriteBuffer(_color02SSBO, uint64_t(), windowSize.x * windowSize.y, GL_DYNAMIC_DRAW);

	// Update size of texture
	glBindTexture(GL_TEXTURE_2D, _textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, windowSize.x, windowSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glGenerateMipmap(GL_TEXTURE_2D);
}
