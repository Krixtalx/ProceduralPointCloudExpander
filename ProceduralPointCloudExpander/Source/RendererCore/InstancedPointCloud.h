#pragma once
#include "stdafx.h"
#include "pointCloud.h"
#include "Utilities/Point.h"


class InstancedPointCloud :public PointCloud {
	std::vector<glm::mat4> offsets;
	unsigned int instancingVBO;
	bool newInstanceUpdate = false;

	void updateInstancingData();

public:

	InstancedPointCloud(std::string shaderProgram, const glm::vec3& pos = { 0, 0, 0 }, const glm::vec3& rot = { 0, 0, 0 }, const glm::vec3& scale = { 1, 1, 1 });

	InstancedPointCloud(std::string shaderProgram, const std::vector<PointModel>& points, const AABB& aabb, const glm::vec3& pos = { 0, 0, 0 }, const glm::vec3& rot = { 0, 0, 0 }, const glm::vec3& scale = { 1, 1, 1 });

	void newInstance(const glm::vec3& position, const glm::vec3& rot, const glm::vec3& scale);

	void drawModel(const glm::mat4& MVPMatrix) override;

	unsigned getNumberOfInstances() const;

};
