#pragma once
#include "stdafx.h"
#include "RendererCore/PointCloud.h"

class PlyLoader {
	static bool writeToBinary(const std::string& filename, PointCloud* pointCloud);
	static PointCloud* readFromBinary(const std::string& filename);
	static void readFromPlyWithClassification(const std::string& _filename);
	static void readFromPlyWithoutClassification(const std::string& _filename);
public:
	static bool saving;
	static unsigned LASClassificationSize;
	static std::unordered_map<std::string, char> LASClassification;
	static std::string LASClassificationStrings[13];

	static void loadPointCloud(const std::string& filename, const bool useClassification = true);
	static void savePointCloud(const std::string& filename, const std::vector<PointCloud*>& clouds);
};