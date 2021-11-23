#pragma once
#include "RendererCore/PointCloud.h"

class PlyLoader
{
	static bool writeToBinary(const std::string& filename, PPCX::PointCloud* pointCloud);
	static PPCX::PointCloud* readFromBinary(const std::string& filename);
	static PPCX::PointCloud* readFromPly(const std::string& _filename);
public:
	static PPCX::PointCloud* loadPointCloud(const std::string& filename);
};