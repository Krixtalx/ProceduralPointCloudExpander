#pragma once
#include "Utilities/Singleton.h"
#include <ProceduralUtils/ProceduralVoxel.h>
class ProceduralGenerator: public Singleton<ProceduralGenerator>{

	friend class Singleton<ProceduralGenerator>;

private:
	PointCloudScene* _pointCloudScene = nullptr;
	PointCloud* pointClouds[2];

	unsigned axisSubdivision[3];
	unsigned axisSubdivisionOriginal[3];
	float gsd;
	int expansion;

	std::vector<std::vector<std::vector<ProceduralVoxel*>>> subdivisions;
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
	void setCurrentCloudScene(PointCloudScene* pointCloudScene);
	

};