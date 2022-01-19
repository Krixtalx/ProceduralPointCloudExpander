#pragma once
#include "Utilities/Singleton.h"
#include <ProceduralUtils/ProceduralVoxel.h>
#include <RendererCore/pointCloud.h>

class ProceduralGenerator {
	friend class GUI;
private:
	unsigned axisSubdivision[2]{};
	float gsd{};

	std::vector<std::vector<ProceduralVoxel*>> subdivisions;
	AABB aabb;
	PointCloud* clouds[2]{};
	float cloudDensity{};
	float progress = 2.0f;

	void readParameters(const std::string& path);
	void meanHeight(unsigned x, unsigned y) const;
	void meanColor(unsigned x, unsigned y) const;
	vec3 getColor(float pointX, float pointY);
	void createVoxelGrid();
	void subdivideCloud();

protected:
	void saveHeightMap(const std::string& path) const;
	void saveTextureMap(const std::string& path) const;
	void savePointCloud(const std::string& path) const;
	void computeNURBS();
	void automaticGSD();
public:
	ProceduralGenerator();
	~ProceduralGenerator();
	void drawClouds(mat4 matrizMVP);
	void newPointCloud(PointCloud* pCloud, bool newScene);
	bool& getPointCloudVisibility(unsigned cloud) const;
};