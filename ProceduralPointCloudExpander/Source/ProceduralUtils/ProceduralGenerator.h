#pragma once
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>

#include "Utilities/Singleton.h"
#include <ProceduralUtils/ProceduralVoxel.h>
#include <RendererCore/pointCloud.h>

class ProceduralGenerator {
	friend class GUI;
protected:
	unsigned axisSubdivision[2]{};
	float gsd{};

	std::vector<std::vector<ProceduralVoxel*>> voxelGrid;
	AABB aabb;
	PointCloud* terrainCloud{};
	float cloudDensity{};
	float progress = FLT_MAX;
	bool useStatiticsMethod = true;


	float getHeight(vec2 pos) const;
	float getDensity(vec2 pos) const;
	void saveHeightMap(const std::string& path) const;
	void saveTextureMap(const std::string& path) const;
	void savePointCloud(const std::string& path) const;
	void automaticGSD(unsigned pointsPerVoxel);
	void generateVoxelGrid(unsigned pointsPerVoxel);
	bool meanNeightbourHeightColor(unsigned x, unsigned y, char minCount) const;
	void createVoxelGrid(std::vector<std::vector<ProceduralVoxel*>>& grid, PointCloud* pointCloud);
	void subdivideCloud(const std::vector<std::vector<ProceduralVoxel*>>& grid, PointCloud* pointCloud);
	void computeHeightAndColor();
	void RegionRGBSegmentation(float distanceThreshold, float pointColorThreshold, float regionColorThreshold, unsigned minClusterSize);
	void RegionRGBSegmentationUsingCloud(const pcl::PointCloud<pcl::PointXYZRGB>::Ptr& cloud, float distanceThreshold, float pointColorThreshold, float regionColorThreshold, unsigned minClusterSize);


	void computeNURBS(unsigned degree, unsigned divX, unsigned divY, float desiredDensityMultiplier);
	void generateProceduralVegetation(const std::vector<std::pair<std::string, std::string>>& data);

	void saveClusterBinary(const std::string& filename, const std::vector<PointCloud*>& clouds);
	bool loadClusterBinary(const std::string& filename) const;

public:
	ProceduralGenerator();
	~ProceduralGenerator();
	void newPointCloud(PointCloud* pCloud, bool newScene, unsigned pointsPerVoxel);
	void testRGBSegmentation();
	static std::vector<std::string> generatedCloudsName;
};