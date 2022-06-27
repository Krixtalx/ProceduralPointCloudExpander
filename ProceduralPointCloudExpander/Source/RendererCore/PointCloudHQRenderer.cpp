#include "stdafx.h"
#include "PointCloudHQRenderer.h"
#include "InstancedPointCloud.h"
#include "ModelManager.h"
#include "Renderer.h"
#include "ShaderManager.h"
#include "RenderOptions.h"

PointCloudHQRenderer::PointCloudHQRenderer() {
	resetDepthBufferHQRShader = PPCX::ShaderManager::getInstancia()->getComputeShader("resetBuffersHQR");
	depthHQRShader = PPCX::ShaderManager::getInstancia()->getComputeShader("depthBufferHQR");
	depthHQRShaderInstancing = PPCX::ShaderManager::getInstancia()->getComputeShader("depthBufferHQRInstancing");
	addColorsHQRShader = PPCX::ShaderManager::getInstancia()->getComputeShader("addColorHQR");
	addColorsHQRShaderInstancing = PPCX::ShaderManager::getInstancia()->getComputeShader("addColorHQRInstancing");
	storeHQRTexture = PPCX::ShaderManager::getInstancia()->getComputeShader("storeTextureHQR");

	windowSize = { PPCX::anchoVentanaPorDefecto, PPCX::altoVentanaPorDefecto };

	_color01SSBO = PPCX::ComputeShader::setWriteBuffer(uint64_t(), windowSize.x * windowSize.y, GL_DYNAMIC_DRAW);
	_color02SSBO = PPCX::ComputeShader::setWriteBuffer(uint64_t(), windowSize.x * windowSize.y, GL_DYNAMIC_DRAW);
	_depthBufferSSBO = PPCX::ComputeShader::setWriteBuffer(uint64_t(), windowSize.x * windowSize.y, GL_DYNAMIC_DRAW);
	_rawDepthBufferSSBO = PPCX::ComputeShader::setWriteBuffer(GLuint(), windowSize.x * windowSize.y, GL_DYNAMIC_DRAW);

	// Window texture
	glGenTextures(1, &_textureID);
	glBindTexture(GL_TEXTURE_2D, _textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, windowSize.x, windowSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glGenerateMipmap(GL_TEXTURE_2D);

	GLuint fboId = 0;
	glGenFramebuffers(1, &fboId);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fboId);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, _textureID, 0);
}

PointCloudHQRenderer::~PointCloudHQRenderer() {
	glDeleteBuffers(1, &_color01SSBO);
	glDeleteBuffers(1, &_color02SSBO);
	glDeleteBuffers(1, &_depthBufferSSBO);
	glDeleteBuffers(1, &_rawDepthBufferSSBO);
	glDeleteTextures(1, &_textureID);
}

void PointCloudHQRenderer::addPointCloud(const std::string& name, PointCloud* pointCloud) {
	const auto previous = pointCloudVBOs.find(name);
	if (previous != pointCloudVBOs.end()) {
		glDeleteBuffers(1, &previous->second);
		const auto previous2 = instancedPointCloudVBOs.find(name);
		if (previous2 != instancedPointCloudVBOs.end())
			glDeleteBuffers(1, &previous2->second);
	}
	GLuint id = PPCX::ComputeShader::setReadBuffer(pointCloud->getPoints(), GL_STATIC_DRAW);
	pointCloudVBOs.insert_or_assign(name, id);
	pointClouds.insert_or_assign(name, pointCloud);

	const auto instancedCloud = dynamic_cast<InstancedPointCloud*>(pointCloud);
	if (instancedCloud) {
		id = PPCX::ComputeShader::setReadBuffer(instancedCloud->getOffsets(), GL_STATIC_DRAW);
		instancedPointCloudVBOs.insert_or_assign(name, id);
	}
}

void PointCloudHQRenderer::render(const mat4& MVPmatrix, const float& distanceThreshold) {
	for (auto& cloud : ModelManager::getInstance()->pendingClouds) {
		addPointCloud(cloud.first, cloud.second);
	}
	ModelManager::getInstance()->pendingClouds.clear();

	const int numGroupsImage = PPCX::ComputeShader::getNumGroups(windowSize.x * windowSize.y);

	// 1. Fill buffer of 32 bits with UINT_MAX
	resetDepthBufferHQRShader->bindBuffers(std::vector<GLuint> { _rawDepthBufferSSBO, _color01SSBO, _color02SSBO });
	PPCX::ShaderManager::getInstancia()->activarSP("ResetComputeShaderSP");
	PPCX::ShaderManager::getInstancia()->setUniform("ResetComputeShaderSP", "windowSize", windowSize);
	resetDepthBufferHQRShader->execute(numGroupsImage, 1, 1, PPCX::ComputeShader::getMaxGroupSize(), 1, 1);

	for (const auto& pointCloudVBO : pointCloudVBOs) {
		const auto pointCloud = pointClouds.find(pointCloudVBO.first)->second;
		if (pointCloud->getVisibility()) {
			const auto instancedCloudVBO = instancedPointCloudVBOs.find(pointCloudVBO.first);

			if (instancedCloudVBO == instancedPointCloudVBOs.end()) {
				const unsigned numPoints = pointCloud->getNumberOfPoints();
				const unsigned numGroupsPoints = PPCX::ComputeShader::getNumGroups(numPoints);
				// 2. Transform points and use atomicMin to retrieve the nearest point
				depthHQRShader->bindBuffers(std::vector{ _rawDepthBufferSSBO, pointCloudVBO.second });
				PPCX::ShaderManager::getInstancia()->activarSP("DepthComputeShaderSP");
				PPCX::ShaderManager::getInstancia()->setUniform("DepthComputeShaderSP", "windowSize", windowSize);
				PPCX::ShaderManager::getInstancia()->setUniform("DepthComputeShaderSP", "numPoints", numPoints);
				PPCX::ShaderManager::getInstancia()->setUniform("DepthComputeShaderSP", "cameraMatrix", MVPmatrix);
				depthHQRShader->execute(numGroupsPoints, 1, 1, PPCX::ComputeShader::getMaxGroupSize(), 1, 1);

				// 3. Accumulate colors once the minimum depth is defined
				addColorsHQRShader->bindBuffers(std::vector<GLuint> { _rawDepthBufferSSBO, _color01SSBO, _color02SSBO, pointCloudVBO.second });
				PPCX::ShaderManager::getInstancia()->activarSP("ColorComputeShaderSP");
				PPCX::ShaderManager::getInstancia()->setUniform("ColorComputeShaderSP", "windowSize", windowSize);
				PPCX::ShaderManager::getInstancia()->setUniform("ColorComputeShaderSP", "numPoints", numPoints);
				PPCX::ShaderManager::getInstancia()->setUniform("ColorComputeShaderSP", "cameraMatrix", MVPmatrix);
				PPCX::ShaderManager::getInstancia()->setUniform("ColorComputeShaderSP", "distanceThreshold", distanceThreshold);
				addColorsHQRShader->execute(numGroupsPoints, 1, 1, PPCX::ComputeShader::getMaxGroupSize(), 1, 1);
			} else {
				const auto instancedPointCloud = dynamic_cast<InstancedPointCloud*>(pointCloud);
				if (instancedPointCloud->HQRNeedUpdate) {
					auto previous = instancedPointCloudVBOs.find(pointCloudVBO.first);
					if (previous != instancedPointCloudVBOs.end())
						glDeleteBuffers(1, &previous->second);
					GLuint id = PPCX::ComputeShader::setReadBuffer(instancedPointCloud->getOffsets(), GL_STATIC_DRAW);
					instancedPointCloudVBOs.insert_or_assign(pointCloudVBO.first, id);
					instancedPointCloud->HQRNeedUpdate = false;

				}
				const unsigned numPoints = instancedPointCloud->getOneInstanceNumberOfPoints();
				const unsigned numInstances = instancedPointCloud->getNumberOfInstances();
				const unsigned numGroupsPoints = PPCX::ComputeShader::getNumGroups(numPoints * numInstances);

				if (numInstances > 0) {
					// 2. Transform points and use atomicMin to retrieve the nearest point
					depthHQRShaderInstancing->bindBuffers(std::vector{ _rawDepthBufferSSBO, pointCloudVBO.second, instancedCloudVBO->second });
					PPCX::ShaderManager::getInstancia()->activarSP("DepthComputeShaderSPInstancing");
					PPCX::ShaderManager::getInstancia()->setUniform("DepthComputeShaderSPInstancing", "windowSize", windowSize);
					PPCX::ShaderManager::getInstancia()->setUniform("DepthComputeShaderSPInstancing", "numPoints", numPoints);
					PPCX::ShaderManager::getInstancia()->setUniform("DepthComputeShaderSPInstancing", "numInstances", numInstances);
					PPCX::ShaderManager::getInstancia()->setUniform("DepthComputeShaderSPInstancing", "cameraMatrix", MVPmatrix);
					depthHQRShaderInstancing->execute(numGroupsPoints, 1, 1, PPCX::ComputeShader::getMaxGroupSize(), 1, 1);


					// 3. Accumulate colors once the minimum depth is defined
					addColorsHQRShaderInstancing->bindBuffers(std::vector<GLuint> { _rawDepthBufferSSBO, _color01SSBO, _color02SSBO, pointCloudVBO.second, instancedCloudVBO->second });
					PPCX::ShaderManager::getInstancia()->activarSP("ColorComputeShaderSPInstancing");
					PPCX::ShaderManager::getInstancia()->setUniform("ColorComputeShaderSPInstancing", "windowSize", windowSize);
					PPCX::ShaderManager::getInstancia()->setUniform("ColorComputeShaderSPInstancing", "numPoints", numPoints);
					PPCX::ShaderManager::getInstancia()->setUniform("ColorComputeShaderSPInstancing", "numInstances", numInstances);
					PPCX::ShaderManager::getInstancia()->setUniform("ColorComputeShaderSPInstancing", "cameraMatrix", MVPmatrix);
					PPCX::ShaderManager::getInstancia()->setUniform("ColorComputeShaderSPInstancing", "distanceThreshold", distanceThreshold);
					addColorsHQRShaderInstancing->execute(numGroupsPoints, 1, 1, PPCX::ComputeShader::getMaxGroupSize(), 1, 1);

				}
			}
		}
	}

	storeHQRTexture->bindBuffers(std::vector{ _color01SSBO, _color02SSBO });
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

void PointCloudHQRenderer::updateWindowSize(const uvec2 newWindowSize) {
	windowSize = newWindowSize;
	PPCX::ComputeShader::updateWriteBuffer(_depthBufferSSBO, uint64_t(), windowSize.x * windowSize.y, GL_DYNAMIC_DRAW);
	PPCX::ComputeShader::updateWriteBuffer(_rawDepthBufferSSBO, GLuint(), windowSize.x * windowSize.y, GL_DYNAMIC_DRAW);
	PPCX::ComputeShader::updateWriteBuffer(_color01SSBO, uint64_t(), windowSize.x * windowSize.y, GL_DYNAMIC_DRAW);
	PPCX::ComputeShader::updateWriteBuffer(_color02SSBO, uint64_t(), windowSize.x * windowSize.y, GL_DYNAMIC_DRAW);

	// Update size of texture
	glBindTexture(GL_TEXTURE_2D, _textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, windowSize.x, windowSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glGenerateMipmap(GL_TEXTURE_2D);
}
