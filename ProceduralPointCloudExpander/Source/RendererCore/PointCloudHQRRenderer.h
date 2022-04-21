#include "stdafx.h"

#include "ComputeShader.h"
#include "pointCloud.h"
#pragma once
class PointCloudHQRRenderer {
	glm::vec2 windowSize;

	//GL buffers
	GLuint _depthBufferSSBO, _rawDepthBufferSSBO, _color01SSBO, _color02SSBO;
	GLuint _textureID;
	// Shaders
	PPCX::ComputeShader* _resetDepthBufferHQRShader;
	PPCX::ComputeShader* _DepthHQRShader;
	PPCX::ComputeShader* _addColorsHQRShader;
	PPCX::ComputeShader* _storeHQRTexture;

	std::map<std::string, GLuint> pointCloudVBOs;
	std::map<std::string, unsigned> pointCloudNpoints;
public:
	~PointCloudHQRRenderer();
	void addPointCloud(const std::string& name, PointCloud* pointCloud);
	void render(const mat4& MVPmatrix);
	void updateWindowSize(const vec2 newWindowSize);
};