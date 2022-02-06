#pragma once
#include "stdafx.h"
#include "RendererCore/PointCloud.h"

class PlyLoader {
	static bool writeToBinary(const std::string& filename, PointCloud* pointCloud);
	static PointCloud* readFromBinary(const std::string& filename);
	static void readFromPly(const std::string& _filename);
public:
	static bool saving;
	static std::unordered_map<std::string, char> LASClassification;
	static std::array<std::string, 13> LASClassificationStrings;

	static void loadPointCloud(const std::string& filename);
	static void savePointCloud(const std::string& filename, const std::vector<PointCloud*>& clouds);
};