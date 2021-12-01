#pragma once
#include "Utilities/Singleton.h"
#include <ProceduralUtils/ProceduralVoxel.h>
#include <RendererCore/pointCloud.h>

class ProceduralGenerator {
private:
	unsigned axisSubdivision[2];
	float gsd;

	std::vector<std::vector<ProceduralVoxel*>> subdivisions;
	AABB aabb;
	PPCX::PointCloud* clouds[2];
	float cloudDensity;

protected:
	void readParameters(const std::string& path);
	void meanHeight(unsigned x, unsigned y);
	void meanColor(unsigned x, unsigned y);
	glm::vec3 getColor(float pointX, float pointY);
	void createVoxelGrid();
	void subdivideCloud();
	void saveHeightMap() const;
	void saveTextureMap();
	void computeNURBS();

public:
	ProceduralGenerator();
	~ProceduralGenerator();
	void drawClouds(glm::mat4 matrizMVP);
	void newPointCloud(PPCX::PointCloud* pCloud, bool newScene);
	bool& getPointCloudVisibility(unsigned cloud);
};