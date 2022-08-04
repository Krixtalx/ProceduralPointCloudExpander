#pragma once
#include "stdafx.h"

#include <RendererCore/pointCloud.h>

class FileManager {
	static void readFromPlyWithClassification(const std::string& filename);
	static void readFromPlyWithoutClassification(const std::string& filename);
public:
	static bool saving;
	static unsigned LASClassificationSize;
	static std::unordered_map<std::string, char> LASClassification;
	static std::string LASClassificationStrings[13];
	static std::vector<std::string> loadedFiles;

	static void loadPointCloud(const std::string& filename, bool useClassification = true);
	static void savePointCloud(const std::string& filename, const std::vector<PointCloud*>& clouds);
	static void loadAllCloudsUnderFolder(const std::string& folder);
};
