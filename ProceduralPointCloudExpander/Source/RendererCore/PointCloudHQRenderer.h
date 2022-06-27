#include "stdafx.h"

#include "ComputeShader.h"
#include "pointCloud.h"
#pragma once
class PointCloudHQRenderer {
	uvec2 windowSize;

	//GL buffers
	GLuint _depthBufferSSBO, _rawDepthBufferSSBO, _color01SSBO, _color02SSBO;
	GLuint _textureID;
	GLuint fboID;

	// Shaders
	PPCX::ComputeShader* resetDepthBufferHQRShader;
	PPCX::ComputeShader* depthHQRShader;
	PPCX::ComputeShader* depthHQRShaderInstancing;
	PPCX::ComputeShader* addColorsHQRShader;
	PPCX::ComputeShader* addColorsHQRShaderInstancing;
	PPCX::ComputeShader* storeHQRTexture;

	std::map<std::string, GLuint> pointCloudVBOs;
	std::map<std::string, GLuint> instancedPointCloudVBOs;
	std::map<std::string, PointCloud*> pointClouds;
public:
	PointCloudHQRenderer();
	~PointCloudHQRenderer();
	void addPointCloud(const std::string& name, PointCloud* pointCloud);
	void render(const mat4& MVPmatrix, const float& distanceThreshold);
	void updateWindowSize(uvec2 newWindowSize);
};