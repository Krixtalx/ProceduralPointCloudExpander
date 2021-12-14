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
	vec3 color = { 0, 0, 0 };

public:

	ProceduralVoxel(PPCX::PointCloud* pointCloud, AABB* aabb);

	~ProceduralVoxel();

	void addPoint(unsigned pointIndex);

	void setAABB(AABB* aabb);

	void computeHeight();

	void computeColor();

	void checkPoints() const;

	bool isInside(PointModel point) const;

	void setHeight(float h);
	void setColor(vec3 color);

	float getHeight() const;

	vec3 getRepresentativePoint() const;

	vec3 getColor() const;

	unsigned getNumberOfPoints() const;

	unsigned numberPointsToDensity(float density) const;
};

