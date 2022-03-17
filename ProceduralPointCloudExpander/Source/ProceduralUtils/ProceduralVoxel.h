#pragma once
#include <GeometryUtils/AABB.h>

#include "Utilities/Point.h"
#include <RendererCore/pointCloud.h>

class ProceduralVoxel {
private:
	AABB* aabb;
	PointCloud* cloud;
	std::vector<unsigned> pointsIndex;
	float height = FLT_MAX;
	vec3 color = { 0, 0, 0 };
	bool vegetationMark = false;

public:

	ProceduralVoxel(PointCloud* pointCloud, AABB* aabb);
	~ProceduralVoxel();
	void addPoint(unsigned pointIndex);
	void setAABB(AABB* aabb);

	void computeHeight();
	void computeColor();

	void checkPoints() const;
	bool isInside(PointModel point) const;

	void setHeight(float h);
	void setColor(vec3 color);
	void setVegetationMark();

	float getHeight() const;
	vec3 getRepresentativePoint() const;
	vec3 getColor() const;
	vec3 getCenter() const;
	unsigned getNumberOfPoints() const;
	float getDensity() const;
	bool getVegetationMark() const;
	AABB getAABB() const;

	unsigned numberPointsToDensity(float density) const;

	std::vector<float> internalDistribution(const unsigned divX, const unsigned divY) const;
};

