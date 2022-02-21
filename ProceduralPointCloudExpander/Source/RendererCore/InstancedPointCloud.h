#pragma once
#include "stdafx.h"
#include "pointCloud.h"


class InstancedPointCloud :public PointCloud {
	std::vector<glm::vec3> offsets;
	unsigned int instancingVBO;
	bool newInstanceUpdate = false;

	void updateInstancingData();

public:

	InstancedPointCloud(std::string shaderProgram, const vec3& pos = { 0, 0, 0 });

	InstancedPointCloud(std::string shaderProgram, const std::vector<PointModel>& points, const AABB& aabb, const vec3& pos = { 0, 0, 0 });

	void newInstance(const glm::vec3& pos);

	void drawModel(const glm::mat4& MVPMatrix) override;

};