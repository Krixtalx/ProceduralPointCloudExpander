#pragma once
#include "Utilities/Singleton.h"
#include <ProceduralUtils/ProceduralVoxel.h>
#include <RendererCore/pointCloud.h>

class ProceduralGenerator: public Singleton<ProceduralGenerator>{

	friend class Singleton<ProceduralGenerator>;

private:
	unsigned axisSubdivision[3];
	float gsd;

	std::vector<std::vector<std::vector<ProceduralVoxel*>>> subdivisions;
	AABB aabb;
	PPCX::PointCloud* nube;
	float cloudDensity;

protected:
	ProceduralGenerator() = default;
	void calculateCloudDensity();
	void readParameters(const std::string & path);
	void meanHeight(unsigned x, unsigned y);
	glm::vec3 getColor(float pointX, float pointY);
	void createVoxelGrid();
	void subdivideCloud();
	void saveHeightMap();
	void saveTextureMap();
	void test();

public:
	~ProceduralGenerator();
};