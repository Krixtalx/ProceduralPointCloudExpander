#pragma once
#include <GeometryUtils/AABB.h>
#include "Utilities/Point.h"
#include <RendererCore/Model.h>
#include "RendererCore/PointCloud.h"

class ProceduralVoxel {
	friend class Quadtree;
private:
	static PointCloud* cloud;
	static unsigned numSubdivisions;

	AABB aabb;
	std::vector<unsigned> pointsIndex;
	std::array<std::shared_ptr<ProceduralVoxel>, 4> children;
	float height = FLT_MAX;
	vec3 color = { 0, 0, 0 };
	bool leaf = true;

	unsigned getDepth(unsigned currentDepth);

	void getAllLeafs(std::vector<std::shared_ptr<ProceduralVoxel>>& leafs) const;

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

	static void setPointCloud(PointCloud* cloud);

	float getHeight() const;

	vec3 getRepresentativePoint() const;

	vec3 getColor() const;

	unsigned getNumberOfPoints() const;

	unsigned numberPointsToDensity(float density) const;

};

