#pragma once
#include <GeometryUtils/AABB.h>

#include "Utilities/Point.h"
#include <RendererCore/pointCloud.h>

class ProceduralVoxel {
private:
	AABB aabb;
	static PPCX::PointCloud* cloud;
	std::vector<unsigned> pointsIndex;
	std::array<std::unique_ptr<ProceduralVoxel>, 4> children;
	float height = FLT_MAX;
	vec3 color = { 0, 0, 0 };
	bool leaf = true;

public:

	ProceduralVoxel(const AABB& aabb);

	~ProceduralVoxel();

	void addPoint(unsigned pointIndex);

	void computeHeight();

	void computeColor();

	void checkPoints() const;

	bool isInside(PointModel point) const;

	void setHeight(float h);

	void setColor(vec3 color);

	static void setPointCloud(PPCX::PointCloud* cloud);

	float getHeight() const;

	vec3 getRepresentativePoint() const;

	vec3 getColor() const;

	unsigned getNumberOfPoints() const;

	unsigned numberPointsToDensity(float density) const;

	unsigned getDepth(unsigned currentDepth);
};

