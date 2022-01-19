#pragma once
#include "RendererCore/PointCloud.h"

class PlyLoader
{
	static bool writeToBinary(const std::string& filename, PointCloud* pointCloud);
	static PointCloud* readFromBinary(const std::string& filename);
	static PointCloud* readFromPly(const std::string& _filename);
public:
	static bool saving;
	static PointCloud* loadPointCloud(const std::string& filename);
	static void savePointCloud(const std::string& filename, const std::vector<PointCloud*>& clouds);
};