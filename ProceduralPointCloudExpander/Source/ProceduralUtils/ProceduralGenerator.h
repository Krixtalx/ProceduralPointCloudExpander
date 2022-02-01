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
	bool useStatiticsMethod = true;

	bool meanNeightbourHeightColor(unsigned x, unsigned y, char minCount) const;
	void createVoxelGrid();
	void subdivideCloud();

protected:
	void saveHeightMap(const std::string& path) const;
	void saveTextureMap(const std::string& path) const;
	void savePointCloud(const std::string& path) const;
	void automaticGSD(unsigned pointsPerVoxel);
	void computeNURBS(unsigned degree, unsigned divX, unsigned divY, float desiredDensityMultiplier);
	void generateVoxelGrid(unsigned pointsPerVoxel);

public:
	ProceduralGenerator();
	~ProceduralGenerator();
	void drawClouds(mat4 matrizMVP);
	void newPointCloud(PointCloud* pCloud, bool newScene, unsigned pointsPerVoxel);
	bool& getPointCloudVisibility(unsigned cloud) const;
};