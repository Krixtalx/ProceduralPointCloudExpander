#include "stdafx.h"
#include "PlyLoader.h"
#include <tinyply/tinyply.h>
#include "Point.h"
#include <filesystem>
#include "RendererCore/ModelManager.h"

constexpr auto PLY_EXTENSION = ".ply";
constexpr auto APPBIN_EXTENSION = ".ppcxbin";

bool PlyLoader::saving = false;
unsigned PlyLoader::LASClassificationSize = 13;

std::unordered_map<std::string, char> PlyLoader::LASClassification{
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

std::string PlyLoader::LASClassificationStrings[] = {
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

bool PlyLoader::writeToBinary(const std::string& filename, PointCloud* pointCloud) {
	std::ofstream fout(filename, std::ios::out | std::ios::binary);
	if (!fout.is_open()) {
		return false;
	}

	const size_t numPoints = pointCloud->getNumberOfPoints();
	fout.write((char*)&numPoints, sizeof(size_t));
	fout.write((char*)&pointCloud->getPoints()[0], numPoints * sizeof(PointModel));
	fout.write((char*)&pointCloud->getAABB(), sizeof(AABB));

	fout.close();

	return true;
}

PointCloud* PlyLoader::readFromBinary(const std::string& filename) {
	std::ifstream fin(filename, std::ios::in | std::ios::binary);
	if (!fin.is_open()) {
		return nullptr;
	}

	size_t numPoints;
	std::vector<PointModel> points;
	AABB _aabb;

	fin.read((char*)&numPoints, sizeof(size_t));
	points.resize(numPoints);
	fin.read((char*)&points[0], numPoints * sizeof(PointModel));
	fin.read((char*)&_aabb, sizeof(AABB));

	fin.close();

	const auto pointCloud = new PointCloud("DefaultSP", points, _aabb);

	return pointCloud;
}

/**
 * Private method for loading a PLY file. It reads ascii and binary ply versions.
 * It will only load points position, color and LAZ classification.
 * 
 * @param _filename PLY filename. Must contain .ply extension in the filename
 */
void PlyLoader::readFromPly(const std::string& _filename) {
	std::vector<uint8_t> byteBuffer;

	try {
		std::vector<PointModel> _points[13];
		std::shared_ptr<tinyply::PlyData> plyColors;
		std::shared_ptr<tinyply::PlyData> plyPoints;
		std::shared_ptr<tinyply::PlyData> plyClassification;

		std::unique_ptr<std::istream> fileStream;
		const std::string& filename = _filename;
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

		unsigned classification = 0;
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
				ModelManager::getInstance()->newModel(PlyLoader::LASClassificationStrings[i], cloud);
			}
		}
	} catch (const std::runtime_error& e) {
		std::cerr << "[PlyLoader::readFromPly]: " << e.what() << std::endl;
	} catch (const std::exception& e) {
		std::cerr << "Caught tinyply exception: " << e.what() << std::endl;
	}
}

/**
 * Public method for loading a point cloud.
 * 
 * @param filename
 */
void PlyLoader::loadPointCloud(const std::string& filename) {
	readFromPly(filename + PLY_EXTENSION);
}


/**
 * Method for saving the generated point cloud
 * 
 * @param filename filename of the generated point cloud
 * @param clouds clouds that will be agregated to form the final point cloud.
 */
void PlyLoader::savePointCloud(const std::string& filename, const std::vector<PointCloud*>& clouds) {
	std::cout << filename << std::endl;
	std::filebuf fb_ascii;
	fb_ascii.open(filename, std::ios::out);
	std::ostream outstream_ascii(&fb_ascii);
	if (outstream_ascii.fail())
		throw std::runtime_error("Failed to open " + filename);

	tinyply::PlyFile file;
	std::vector<vec3> points;
	std::vector<vec3> colors;
	saving = true;
	for (const auto cloud : clouds) {
		auto& pointModels = cloud->getPoints();
		for (auto& pointModel : pointModels) {
			points.push_back(pointModel._point);
			const vec4 aux = pointModel.getRGBVec3() / 255.0f;
			colors.emplace_back(vec3(aux.r, aux.g, aux.b));
		}
	}
	file.add_properties_to_element("vertex", { "x", "y", "z" },
								   tinyply::Type::FLOAT32, points.size(), reinterpret_cast<uint8_t*>(points.data()), tinyply::Type::INVALID, 0);
	file.add_properties_to_element("vertex", { "red", "green", "blue" },
								   tinyply::Type::FLOAT32, colors.size(), reinterpret_cast<uint8_t*>(colors.data()), tinyply::Type::INVALID, 0);
	file.get_comments().emplace_back("generated by PPCX");

	file.write(outstream_ascii, false);
	fb_ascii.close();
	saving = false;
}
