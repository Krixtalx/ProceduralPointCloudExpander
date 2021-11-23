#pragma once
#include <GeometryUtils/AABB.h>

#include "Utilities/Point.h"
#include <RendererCore/pointCloud.h>

class ProceduralVoxel {
private:
	AABB* aabb;
	PPCX::PointCloud* nube;
	std::vector<unsigned> pointsIndex;
	float height = FLT_MAX;
	glm::vec3 color = { 0, 0, 0 };

public:

	ProceduralVoxel(PPCX::PointCloud* pointCloud, AABB* aabb);

	~ProceduralVoxel();

	void addPoint(unsigned pointIndex);

	void setAABB(AABB* aabb);

	void computeHeight();

	void computeColor();

	void checkPoints();

	bool isInside(PointModel point);

	void setHeight(float h);

	float getHeight();

	glm::vec3 getMidPoint();

	glm::vec3 getColor();
};

