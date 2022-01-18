#pragma once
#include "RendererCore/Model.h"

class PointCloud;

class PlyLoader
{
	static bool writeToBinary(const std::string& filename, PointCloud* pointCloud);
	static PointCloud* readFromBinary(const std::string& filename);
	static PointCloud* readFromPly(const std::string& _filename);
public:
	static bool saving;
	static PointCloud* loadPointCloud(const std::string& filename);
	static void savePointCloud(const std::string& filename, std::vector<PointCloud*> clouds);
};
