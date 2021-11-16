#include "stdafx.h"
#include "PlyLoader.h"
#include <tinyply/tinyply.h>
#include "Point.h"

PAG::pointCloud* PlyLoader::cargarModelo(std::string _filename)
{
	std::unique_ptr<std::istream> fileStream;
	std::vector<uint8_t> byteBuffer;
	std::shared_ptr<tinyply::PlyData> plyPoints, plyColors;
	std::vector<PointModel> _points;
	unsigned baseIndex;
	float* pointsRawFloat = nullptr;
	double* pointsRawDouble = nullptr;
	uint8_t* colorsRaw;

	try
	{
		const std::string filename = _filename + ".ply";
		fileStream.reset(new std::ifstream(filename, std::ios::binary));

		if (!fileStream || fileStream->fail()) return nullptr;

		fileStream->seekg(0, std::ios::end);
		const float size_mb = fileStream->tellg() * float(1e-6);
		fileStream->seekg(0, std::ios::beg);

		tinyply::PlyFile file;
		file.parse_header(*fileStream);

		try { plyPoints = file.request_properties_from_element("vertex", { "x", "y", "z" }); }
		catch (const std::exception& e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

		try { plyColors = file.request_properties_from_element("vertex", { "red", "green", "blue" }); }
		catch (const std::exception& e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

		file.read(*fileStream);

		{
			const bool isDouble = plyPoints->t == tinyply::Type::FLOAT64;
			const size_t numPoints = plyPoints->count;
			const size_t numPointsBytes = numPoints * (!isDouble ? sizeof(float) : sizeof(double)) * 3;

			const size_t numColors = plyColors->count;
			const size_t numColorsBytes = numColors * 1 * 3;

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
			colorsRaw = new uint8_t[numColors * 3];

			std::memcpy(colorsRaw, plyColors->buffer.get(), numColorsBytes);

			if (!isDouble)
			{
				for (unsigned index = 0; index < numPoints; ++index)
				{
					baseIndex = index * 3;

					_points[index] = PointModel{ glm::vec3(pointsRawFloat[baseIndex], pointsRawFloat[baseIndex + 1], pointsRawFloat[baseIndex + 2]),PointModel::getRGBColor(glm::vec3(colorsRaw[baseIndex], colorsRaw[baseIndex + 1], colorsRaw[baseIndex + 2])) };
					//_aabb.update(_points[index]._point);
				}
			}
			else
			{
				for (unsigned index = 0; index < numPoints; ++index)
				{
					baseIndex = index * 3;

					_points[index] = PointModel{ glm::vec3(pointsRawDouble[baseIndex], pointsRawDouble[baseIndex + 1], pointsRawDouble[baseIndex + 2]),
												 PointModel::getRGBColor(glm::vec3(colorsRaw[baseIndex], colorsRaw[baseIndex + 1], colorsRaw[baseIndex + 2])) };
					//_aabb.update(_points[index]._point);
				}
			}
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Caught tinyply exception: " << e.what() << std::endl;

		return nullptr;
	}
	PAG::pointCloud* nube = new PAG::pointCloud("DefaultSP");
	nube->nuevoVBO(_points, GL_STATIC_DRAW);
	std::vector<unsigned> ibo;
	ibo.resize(_points.size());
	std::iota(ibo.begin(), ibo.end(), 0);
	nube->nuevoIBO(ibo, GL_STATIC_DRAW);
	return nube;
}
