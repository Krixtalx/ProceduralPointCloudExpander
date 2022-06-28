#pragma once
#include "stdafx.h"
#include "pointCloud.h"
#include "Utilities/Point.h"


class InstancedPointCloud :public PointCloud {
	std::vector<mat4> offsets;
	unsigned int instancingVBO;
	bool newInstanceUpdate = false;
	std::vector<PointModel> allPoints;
	void updateInstancingData();

public:

	bool HQRNeedUpdate = false;

	InstancedPointCloud(std::string shaderProgram, const vec3& pos = { 0, 0, 0 }, const vec3& rot = { 0, 0, 0 }, const
						vec3& scale = { 1, 1, 1 });

	InstancedPointCloud(std::string shaderProgram, const std::vector<PointModel>& points, const AABB& aabb, const vec3& pos = { 0, 0, 0 }, const
						vec3& rot = { 0, 0, 0 }, const vec3& scale = { 1, 1, 1 });

	void newInstance(const vec3& position, const vec3& rot, const vec3& scale);

	void drawModel(const mat4& MVPMatrix) override;

	void clearInstances();

	std::vector<PointModel>& getAllInstancesPoints();

	std::vector<mat4>& getOffsets();

	unsigned getNumberOfInstances() const;

	unsigned getNumberOfPoints() const override;

	unsigned getOneInstanceNumberOfPoints() const;
};
