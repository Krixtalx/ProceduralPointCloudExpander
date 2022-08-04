#include "stdafx.h"
#include "FileManager.h"
#include <tinyply/tinyply.h>
#include "Point.h"
#include <filesystem>
#include "RendererCore/ModelManager.h"
#include "RendererCore/InstancedPointCloud.h"

constexpr auto PLY_EXTENSION = ".ply";

bool FileManager::saving = false;
unsigned FileManager::LASClassificationSize = 13;

std::unordered_map<std::string, char> FileManager::LASClassification{
	{"Never Classified", 0},
	{"Unclassified", 1},
	{"Ground", 2},
	{"Low vegetation", 3},
	{"Medium vegetation", 4},
	{"High vegetation", 5},
	{"Building", 6},
	{"Low point noise", 7},
	{"Model Key-Point", 8},
	{"Water", 9},
	{"Rail", 10},
	{"Road surface", 11},
	{"Overlaped points", 12}
};

std::string FileManager::LASClassificationStrings[] = {
	"Never Classified",
	"Unclassified",
	"Ground",
	"Low vegetation",
	"Medium vegetation",
	"High vegetation",
	"Building",
	"Low point noise",
	"Model Key-Point",
	"Water",
	"Rail",
	"Road surface",
	"Overlaped points"
};

/**
 * Private method for loading a PLY file. It reads ascii and binary ply versions.
 * It will only load points position, color and LAZ classification.
 *
 * @param filename PLY filename. Must contain .ply extension in the filename
 */
void FileManager::readFromPlyWithClassification(const std::string& filename) {
	std::vector<uint8_t> byteBuffer;

	try {
		std::vector<PointModel> _points[13];
		std::shared_ptr<tinyply::PlyData> plyColors;
		std::shared_ptr<tinyply::PlyData> plyPoints;
		std::shared_ptr<tinyply::PlyData> plyClassification;

		std::unique_ptr<std::istream> fileStream;
		bool haveColors = true;
		bool haveClassification = true;

		fileStream.reset(new std::ifstream(filename, std::ios::binary));

		if (!fileStream || fileStream->fail()) throw std::runtime_error("Unexpected error while trying to load the file");

		fileStream->seekg(0, std::ios::end);
		fileStream->seekg(0, std::ios::beg);

		tinyply::PlyFile file;
		file.parse_header(*fileStream);

		try {
			plyPoints = file.request_properties_from_element("vertex", { "x", "y", "z" });
		} catch (const std::exception& e) {
			std::cerr << "tinyply exception: " << e.what() << std::endl;
		}

		try {
			plyColors = file.request_properties_from_element("vertex", { "red", "green", "blue" });
		} catch (const std::invalid_argument& e) {
			std::cerr << "tinyply exception: " << e.what() << std::endl;
			haveColors = false;
		}

		try {
			plyClassification = file.request_properties_from_element("vertex", { "scalar_Classification" });
		} catch (const std::invalid_argument& e) {
			std::cerr << "tinyply exception: " << e.what() << std::endl;
			haveClassification = false;
		}

		file.read(*fileStream);

		double* pointsRawDouble = nullptr;
		float* pointsRawFloat = nullptr;

		unsigned baseIndex;
		const bool isDouble = plyPoints->t == tinyply::Type::FLOAT64;
		const size_t numPoints = plyPoints->count;
		const size_t numPointsBytes = numPoints * (!isDouble ? sizeof(float) : sizeof(double)) * 3;
		size_t numColors;

		if (haveColors)
			numColors = plyColors->count;
		else
			numColors = plyPoints->count;
		const size_t numColorsBytes = numColors * 1 * 3;

		AABB _aabb[13];

		std::cout << "Number of points: " << numPoints << std::endl;


		if (!isDouble) {
			pointsRawFloat = new float[numPoints * 3];
			std::memcpy(pointsRawFloat, plyPoints->buffer.get(), numPointsBytes);
		} else {
			pointsRawDouble = new double[numPoints * 3];
			std::memcpy(pointsRawDouble, plyPoints->buffer.get(), numPointsBytes);
		}
		const auto colorsRaw = new uint8_t[numColors * 3];

		if (haveColors)
			std::memcpy(colorsRaw, plyColors->buffer.get(), numColorsBytes);
		else {
			for (int i = 0; i < numColors * 3; ++i) {
				colorsRaw[i] = UINT8_MAX;
			}
		}

		const auto classificationRaw = new float[numPoints];
		if (haveClassification)
			std::memcpy(classificationRaw, plyClassification->buffer.get(), numPointsBytes / 3);
		else {
			for (int i = 0; i < numPoints; ++i) {
				classificationRaw[i] = 2;
			}
		}

		unsigned classification;
		if (!isDouble) {
			for (unsigned index = 0; index < numPoints; ++index) {
				baseIndex = index * 3;
				classification = classificationRaw[index] + 0.5f;
				_points[classification].push_back(PointModel{
					vec3(pointsRawFloat[baseIndex], pointsRawFloat[baseIndex + 1],
						 pointsRawFloat[baseIndex + 2]),
					PointModel::getRGBColor(vec3(colorsRaw[baseIndex], colorsRaw[baseIndex + 1],
												 colorsRaw[baseIndex + 2]))
					});
				_aabb[classification].update(_points[classification].back()._point);
			}
		} else {
			for (unsigned index = 0; index < numPoints; ++index) {
				baseIndex = index * 3;
				classification = classificationRaw[index] + 0.5f;
				_points[classification].push_back(PointModel{
					vec3(pointsRawDouble[baseIndex], pointsRawDouble[baseIndex + 1],
						 pointsRawDouble[baseIndex + 2]),
					PointModel::getRGBColor(vec3(colorsRaw[baseIndex], colorsRaw[baseIndex + 1],
												 colorsRaw[baseIndex + 2]))
					});
				_aabb[classification].update(_points[classification].back()._point);
			}
		}

		for (size_t i = 0; i < 13; i++) {
			if (_points[i].size() > 100) {
				const auto cloud = new PointCloud("DefaultSP", _points[i], _aabb[i]);
				cloud->optimize();
				cloud->classification = LASClassificationStrings[i];
				ModelManager::getInstance()->modifyModel(LASClassificationStrings[i], cloud);
			}
		}
	} catch (const std::runtime_error& e) {
		std::cerr << "[FileManager::readFromPly]: " << e.what() << std::endl;
	} catch (const std::exception& e) {
		std::cerr << "Caught tinyply exception: " << e.what() << std::endl;
	}
}

void FileManager::readFromPlyWithoutClassification(const std::string& filename) {
	std::vector<uint8_t> byteBuffer;

	try {
		std::vector<PointModel> _points;
		std::shared_ptr<tinyply::PlyData> plyColors;
		std::shared_ptr<tinyply::PlyData> plyPoints;

		std::unique_ptr<std::istream> fileStream;
		bool haveColors = true;
		bool haveClassification = true;

		fileStream.reset(new std::ifstream(filename, std::ios::binary));

		if (!fileStream || fileStream->fail()) throw std::runtime_error("Unexpected error while trying to load the file");

		fileStream->seekg(0, std::ios::end);
		fileStream->seekg(0, std::ios::beg);

		tinyply::PlyFile file;
		file.parse_header(*fileStream);

		try {
			plyPoints = file.request_properties_from_element("vertex", { "x", "y", "z" });
		} catch (const std::exception& e) {
			std::cerr << "tinyply exception: " << e.what() << std::endl;
		}

		try {
			plyColors = file.request_properties_from_element("vertex", { "red", "green", "blue" });
		} catch (const std::invalid_argument& e) {
			std::cerr << "tinyply exception: " << e.what() << std::endl;
			haveColors = false;
		}

		file.read(*fileStream);

		double* pointsRawDouble = nullptr;
		float* pointsRawFloat = nullptr;

		unsigned baseIndex;
		const bool isDouble = plyPoints->t == tinyply::Type::FLOAT64;
		const size_t numPoints = plyPoints->count;
		const size_t numPointsBytes = numPoints * (!isDouble ? sizeof(float) : sizeof(double)) * 3;
		size_t numColors;

		if (haveColors)
			numColors = plyColors->count;
		else
			numColors = plyPoints->count;
		const size_t numColorsBytes = numColors * 1 * 3;

		AABB _aabb;

		std::cout << "Number of points: " << numPoints << std::endl;


		if (!isDouble) {
			pointsRawFloat = new float[numPoints * 3];
			std::memcpy(pointsRawFloat, plyPoints->buffer.get(), numPointsBytes);
		} else {
			pointsRawDouble = new double[numPoints * 3];
			std::memcpy(pointsRawDouble, plyPoints->buffer.get(), numPointsBytes);
		}
		const auto colorsRaw = new uint8_t[numColors * 3];

		if (haveColors)
			std::memcpy(colorsRaw, plyColors->buffer.get(), numColorsBytes);
		else {
			for (int i = 0; i < numColors * 3; ++i) {
				colorsRaw[i] = UINT8_MAX;
			}
		}

		unsigned classification = 0;
		_points.reserve(numPoints);
		if (!isDouble) {
			for (unsigned index = 0; index < numPoints; ++index) {
				baseIndex = index * 3;
				_points.push_back(PointModel{
					vec3(pointsRawFloat[baseIndex], pointsRawFloat[baseIndex + 1],
						 pointsRawFloat[baseIndex + 2]),
					PointModel::getRGBColor(vec3(colorsRaw[baseIndex], colorsRaw[baseIndex + 1],
												 colorsRaw[baseIndex + 2]))
					});
				_aabb.update(_points.back()._point);
			}
		} else {
			for (unsigned index = 0; index < numPoints; ++index) {
				baseIndex = index * 3;
				_points.push_back(PointModel{
					vec3(pointsRawDouble[baseIndex], pointsRawDouble[baseIndex + 1],
						 pointsRawDouble[baseIndex + 2]),
					PointModel::getRGBColor(vec3(colorsRaw[baseIndex], colorsRaw[baseIndex + 1],
												 colorsRaw[baseIndex + 2]))
					});
				_aabb.update(_points.back()._point);
			}
		}
		const auto cloud = new InstancedPointCloud("InstancingSP", _points, _aabb);
		cloud->classification = "High vegetation";
		std::string name = filename.substr(filename.find_last_of('\\') + 1);
		name.erase(name.length() - 4, name.length());
		ModelManager::getInstance()->newModel(name, cloud);
		std::cout << name << ": " << _aabb.size().x << "-" << _aabb.size().y << std::endl;
	} catch (const std::runtime_error& e) {
		std::cerr << "[FileManager::readFromPly]: " << e.what() << std::endl;
	} catch (const std::exception& e) {
		std::cerr << "Caught tinyply exception: " << e.what() << std::endl;
	}
}

/**
 * Public method for loading a point cloud.
 *
 * @param filename
 * @param useClassification
 */
void FileManager::loadPointCloud(const std::string& filename, const bool useClassification) {
	std::cout << "Loading point cloud: " << filename << std::endl;
	if (useClassification)
		readFromPlyWithClassification(filename + PLY_EXTENSION);
	else
		readFromPlyWithoutClassification(filename + PLY_EXTENSION);
}


/**
 * Method for saving the generated point cloud
 *
 * @param filename filename of the generated point cloud
 * @param clouds clouds that will be agregated to form the final point cloud.
 */
void FileManager::savePointCloud(const std::string& filename, const std::vector<PointCloud*>& clouds) {
	std::filebuf fb_binary;
	fb_binary.open(filename, std::ios::out | std::ios::binary);
	std::ostream outstream_binary(&fb_binary);
	if (outstream_binary.fail())
		throw std::runtime_error("failed to open " + filename);

	tinyply::PlyFile file;
	std::vector<vec3> points;
	std::vector<vec3> colors;
	std::vector<uint> classification;
	saving = true;

	uint totalPointNumber = 0;
	for (const auto cloud : clouds) {
		if (cloud->getVisibility())
			totalPointNumber += cloud->getNumberOfPoints();
	}
	points.reserve(totalPointNumber);
	colors.reserve(totalPointNumber);
	classification.reserve(totalPointNumber);

	for (const auto cloud : clouds) {
		if (cloud->getVisibility()) {
			const uint classificationID = LASClassification.find(cloud->classification)->second;
			const auto instancedCloud = dynamic_cast<InstancedPointCloud*>(cloud);

			if (instancedCloud) {
				const auto& pointModels = instancedCloud->getAllInstancesPoints();
				for (auto& pointModel : pointModels) {
					points.push_back(pointModel._point);
					colors.emplace_back(pointModel.getRGBVec3() / 255.0f);
					classification.push_back(classificationID);
				}
			} else {
				const auto& pointModels = cloud->getPoints();
				for (auto& pointModel : pointModels) {
					points.push_back(pointModel._point);
					colors.emplace_back(pointModel.getRGBVec3() / 255.0f);
					classification.push_back(classificationID);
				}
			}
		}
	}
	file.add_properties_to_element("vertex", { "x", "y", "z" },
		tinyply::Type::FLOAT32, points.size(), reinterpret_cast<uint8_t*>(points.data()), tinyply::Type::INVALID, 0);
	file.add_properties_to_element("vertex", { "red", "green", "blue" },
		tinyply::Type::FLOAT32, colors.size(), reinterpret_cast<uint8_t*>(colors.data()), tinyply::Type::INVALID, 0);
	file.add_properties_to_element("vertex", { "scalar_Classification" },
		tinyply::Type::UINT32, classification.size(), reinterpret_cast<uint8_t*>(classification.data()), tinyply::Type::INVALID, 0);

	file.get_comments().emplace_back("generated by PPCX");

	file.write(outstream_binary, true);
	fb_binary.close();
	saving = false;
}

void FileManager::loadAllCloudsUnderFolder(const std::string& folder) {
	for (const auto& file : std::filesystem::directory_iterator(folder)) {
		std::string filename = file.path().string();
		std::string extension = filename.substr(filename.find_last_of('.'), 4);
		if (extension == ".ply") {
			filename.erase(filename.find_last_of('.'), 4);
			loadPointCloud(filename, false);
		}
	}
}