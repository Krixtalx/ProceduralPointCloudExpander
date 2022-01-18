#pragma once
#include "Utilities/Singleton.h"
#include <ProceduralUtils/ProceduralVoxel.h>
#include "Utilities/Quadtree.h"
#include <RendererCore/PointCloud.h>

class ProceduralGenerator {
	friend class GUI;
private:
	Quadtree* quadtree = nullptr;
	AABB aabb;
	PointCloud* clouds[2]{};
	std::vector<std::shared_ptr<ProceduralVoxel>> leafs;
	float cloudDensity{};
	float progress = 2.0f;

	//void readParameters(const std::string& path);
	void subdivideCloud();

protected:
	//void saveHeightMap(std::string path) const;
	//void saveTextureMap(std::string path) const;
	void savePointCloud(std::string path) const;
	void computeNURBS();

public:
	ProceduralGenerator();
	~ProceduralGenerator();
	void drawClouds(mat4 matrizMVP);
	void newPointCloud(PointCloud* pCloud, bool newScene);
	bool& getPointCloudVisibility(unsigned cloud) const;
};
