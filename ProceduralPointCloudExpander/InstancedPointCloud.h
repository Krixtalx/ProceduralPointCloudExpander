#pragma once
#include "stdafx.h"
#include "pointCloud.h"


class InstancedPointCloud :public PointCloud {
	std::vector<glm::vec3> offsets;
	unsigned int instancingVBO;
	bool newInstance = false;

public:

	InstancedPointCloud();

};