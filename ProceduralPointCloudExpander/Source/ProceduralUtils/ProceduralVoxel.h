#pragma once
#include <GeometryUtils/AABB.h>

#include "Utilities/Point.h"

namespace PPCX
{
	class PointCloud;
}

class ProceduralVoxel{
private:
	AABB* aabb;
	PPCX::PointCloud* pointCloud;
	std::vector<unsigned> pointsIndex;
	bool procedural = true;

	float height;
	glm::vec3 color;

public:

	ProceduralVoxel(PPCX::PointCloud* pointCloud, AABB* aabb);

	~ProceduralVoxel();

	void addPoint(unsigned pointIndex);
	
	void setAABB(AABB* aabb);

	void setProcedural(bool proc);

	void computeHeight();

	void computeColor();
	
	void checkPoints();

	bool isInside(PointModel point);

	bool load(const glm::mat4& modelMatrix);

	void setHeight(float h);

	float getHeight();

	glm::vec3 getMidPoint();

	glm::vec3 getColor();
};

