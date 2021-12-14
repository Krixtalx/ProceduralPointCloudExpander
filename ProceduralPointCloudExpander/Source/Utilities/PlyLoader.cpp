#include "stdafx.h"
#include "PlyLoader.h"
#include <tinyply/tinyply.h>
#include "Point.h"
#include <filesystem>

constexpr auto PLY_EXTENSION = ".ply";
constexpr auto APPBIN_EXTENSION = ".ppcxbin";

bool PlyLoader::writeToBinary(const std::string& filename, PPCX::PointCloud* pointCloud)
{
	std::ofstream fout(filename, std::ios::out | std::ios::binary);
	if (!fout.is_open())
	{
		return false;
	}

	const size_t numPoints = pointCloud->getNumberOfPoints();
	fout.write((char*)&numPoints, sizeof(size_t));
	fout.write((char*)&pointCloud->getPoints()[0], numPoints * sizeof(PointModel));
	fout.write((char*)&pointCloud->getAABB(), sizeof(AABB));

	fout.close();

	return true;
}

PPCX::PointCloud* PlyLoader::readFromBinary(const std::string& filename)
{
	std::ifstream fin(filename, std::ios::in | std::ios::binary);
	if (!fin.is_open())
	{
		return nullptr;
	}

	size_t numPoints;
	PPCX::PointCloud* pointCloud;
	std::vector<PointModel> points;
	AABB _aabb;

	fin.read((char*)&numPoints, sizeof(size_t));
	points.resize(numPoints);
	fin.read((char*)&points[0], numPoints * sizeof(PointModel));
	fin.read((char*)&_aabb, sizeof(AABB));

	fin.close();

	pointCloud = new PPCX::PointCloud("DefaultSP", points, _aabb);

	return pointCloud;
}

PPCX::PointCloud* PlyLoader::readFromPly(const std::string& _filename)
{
	std::vector<uint8_t> byteBuffer;
	std::vector<PointModel> _points;

	try
	{
		std::shared_ptr<tinyply::PlyData> plyColors;
		std::shared_ptr<tinyply::PlyData> plyPoints;
		std::unique_ptr<std::istream> fileStream;
		const std::string filename = _filename;
		fileStream.reset(new std::ifstream(filename, std::ios::binary));

		if (!fileStream || fileStream->fail()) return nullptr;

		fileStream->seekg(0, std::ios::end);
		fileStream->seekg(0, std::ios::beg);

		tinyply::PlyFile file;
		file.parse_header(*fileStream);

		try { plyPoints = file.request_properties_from_element("vertex", { "x", "y", "z" }); }
		catch (const std::exception& e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

		try { plyColors = file.request_properties_from_element("vertex", { "red", "green", "blue" }); }
		catch (const std::exception& e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

		file.read(*fileStream);


		double* pointsRawDouble = nullptr;
		float* pointsRawFloat = nullptr;
		unsigned baseIndex;
		const bool isDouble = plyPoints->t == tinyply::Type::FLOAT64;
		const size_t numPoints = plyPoints->count;
		const size_t numPointsBytes = numPoints * (!isDouble ? sizeof(float) : sizeof(double)) * 3;

		const size_t numColors = plyColors->count;
		const size_t numColorsBytes = numColors * 1 * 3;

		AABB _aabb;

		// Allocate space
		_points.resize(numPoints);
		if (!isDouble)
		{
			pointsRawFloat = new float[numPoints * 3];
			std::memcpy(pointsRawFloat, plyPoints->buffer.get(), numPointsBytes);
		}
		else
		{
			pointsRawDouble = new double[numPoints * 3];
			std::memcpy(pointsRawDouble, plyPoints->buffer.get(), numPointsBytes);
		}
		const auto colorsRaw = new uint8_t[numColors * 3];

		std::memcpy(colorsRaw, plyColors->buffer.get(), numColorsBytes);

		if (!isDouble)
		{
			for (unsigned index = 0; index < numPoints; ++index)
			{
				baseIndex = index * 3;

				_points[index] = PointModel{
					vec3(pointsRawFloat[baseIndex], pointsRawFloat[baseIndex + 1],
					     pointsRawFloat[baseIndex + 2]),
					PointModel::getRGBColor(vec3(colorsRaw[baseIndex], colorsRaw[baseIndex + 1],
					                             colorsRaw[baseIndex + 2]))
				};
				_aabb.update(_points[index]._point);
			}
		}
		else
		{
			for (unsigned index = 0; index < numPoints; ++index)
			{
				baseIndex = index * 3;

				_points[index] = PointModel{
					vec3(pointsRawDouble[baseIndex], pointsRawDouble[baseIndex + 1],
					     pointsRawDouble[baseIndex + 2]),
					PointModel::getRGBColor(vec3(colorsRaw[baseIndex], colorsRaw[baseIndex + 1],
					                             colorsRaw[baseIndex + 2]))
				};
				_aabb.update(_points[index]._point);
			}
		}

		auto nube = new PPCX::PointCloud("DefaultSP", _points, _aabb);
		return nube;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Caught tinyply exception: " << e.what() << std::endl;

		return nullptr;
	}

	return nullptr;
}

PPCX::PointCloud* PlyLoader::loadPointCloud(const std::string& filename)
{
	std::string file = filename + APPBIN_EXTENSION;
	if (std::filesystem::exists(file)) {
		return readFromBinary(file);
	}

	PPCX::PointCloud* pointCloud = readFromPly(filename + PLY_EXTENSION);
	if (pointCloud)
		writeToBinary(file, pointCloud);
	return pointCloud;
}
