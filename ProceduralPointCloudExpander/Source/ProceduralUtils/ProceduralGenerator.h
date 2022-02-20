#pragma once
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>

#include "Utilities/Singleton.h"
#include <ProceduralUtils/ProceduralVoxel.h>
#include <RendererCore/pointCloud.h>

class ProceduralGenerator {
	friend class GUI;
private:
	unsigned axisSubdivision[2]{};
	float gsd{};

	std::vector<std::vector<ProceduralVoxel*>> voxelGrid;
	AABB aabb;
	PointCloud* terrainCloud{};
	float cloudDensity{};
	float progress = FLT_MAX;
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
	void RegionRGBSegmentation(float distanceThreshold, float pointColorThreshold, float regionColorThreshold, unsigned minClusterSize);
	void RegionRGBSegmentationUsingCloud(const pcl::PointCloud<pcl::PointXYZRGB>::Ptr& cloud, float distanceThreshold, float pointColorThreshold, float regionColorThreshold, unsigned minClusterSize);

public:
	ProceduralGenerator();
	~ProceduralGenerator();
	void newPointCloud(PointCloud* pCloud, bool newScene, unsigned pointsPerVoxel);
	void testRGBSegmentation();
	static std::vector<std::string> generatedCloudsName;
};