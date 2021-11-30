#include "stdafx.h"
#include "ProceduralGenerator.h"
#include "tinynurbs.h"
#include "Utilities/Image.h"

ProceduralGenerator::ProceduralGenerator() = default;

ProceduralGenerator::~ProceduralGenerator() {
	for (size_t x = 0; x < axisSubdivision[0]; x++) {
		for (size_t y = 0; y < axisSubdivision[1]; y++) {
			delete subdivisions[x][y];
		}
	}
	for (const auto& cloud : clouds) {
		delete cloud;
	}
}

void ProceduralGenerator::drawClouds(glm::mat4 matrizMVP) {
	for (const auto& cloud : clouds) {
		if (cloud)
			cloud->dibujarModelo(matrizMVP);
	}
}

void ProceduralGenerator::newPointCloud(PPCX::PointCloud * pCloud, bool newScene) {
	for (size_t x = 0; x < axisSubdivision[0]; x++) {
		for (size_t y = 0; y < axisSubdivision[1]; y++) {
			delete subdivisions[x][y];
		}
	}
	delete clouds[1];
	clouds[1] = nullptr;
	if (newScene || !clouds[0]) {
		for (const auto& cloud : clouds) {
			delete cloud;
		}
		clouds[0] = pCloud;
	} else {
		const auto vec = pCloud->getPoints();
		for (auto i : vec) {
			clouds[0]->nuevoPunto(i);
		}
		clouds[0]->actualizarNube();
	}
	this->aabb = clouds[0]->getAABB();
	readParameters("proceduralParameters.ini");
	createVoxelGrid();
	subdivideCloud();
	test();
}


/**
 * @brief Read the parameters from the parameters file
 * @param path where the parameters file is
*/
void ProceduralGenerator::readParameters(const std::string & path) {
	std::ifstream parametersFile;
	parametersFile.open(path, std::ifstream::in);

	if (!parametersFile) {
		std::cerr << "[ProceduralGenerator]->Error while trying to open the parameters file. Path: " + path << std::endl;
	} else {
		std::string line;
		while (!parametersFile.eof()) {
			getline(parametersFile, line);
			if (!line.empty()) {
				int pos = line.find("=");
				std::string parameter = line.substr(0, pos);
				std::string value = line.substr(++pos, line.length());
				if (parameter == "gsd")	gsd = stof(value);
			}
		}
		parametersFile.close();
		glm::vec3 size = aabb.size();
		for (size_t i = 0; i < 2; i++) {
			axisSubdivision[i] = round(size[i] / gsd);
		}
		//axisSubdivisionOriginal[0] = 1000;
		//axisSubdivisionOriginal[1] = 500;
	}
}

/**
 * @brief compute the height of a proceduralVoxel as the mean of the inmediate neightbour
 * @param x index of the subdivision
 * @param y index of the subdivision
*/
void ProceduralGenerator::meanHeight(unsigned x, unsigned y) {
	float mean = 0;
	char counter = 0;
	for (int i = -1; i < 2; i++) {
		for (int j = -1; j < 2; j++) {
			int auxX = x + i;
			auxX = std::min((int)axisSubdivision[0] - 1, std::max(0, auxX));
			int auxY = y + j;
			auxY = std::min((int)axisSubdivision[1] - 1, std::max(0, auxY));
			if (auxX != x || auxY != y) {
				counter++;
				const float height = subdivisions[auxX][auxY]->getHeight();
				if (height != FLT_MAX)
					mean += height;
				else
					counter--;
			}
		}
	}
	if (counter > 0) {
		mean /= counter;
		subdivisions[x][y]->setHeight(mean);
	}
}


/**
 * @brief Initialize the voxel grid
*/
void ProceduralGenerator::createVoxelGrid() {
	std::cout << "Creating voxel grid..." << std::endl;
	glm::vec3 size = aabb.size();
	const glm::vec3 minPoint = aabb.min();
	float stride[3];

	for (unsigned i = 0; i < 2; i++) {
		stride[i] = size[i] / axisSubdivision[i];
	}
	stride[2] = size[2];

	subdivisions.resize(axisSubdivision[0]);
	for (size_t x = 0; x < axisSubdivision[0]; x++) {
		subdivisions[x].resize(axisSubdivision[1]);
		for (size_t y = 0; y < axisSubdivision[1]; y++) {
			const auto newAABB = new AABB;
			glm::vec3 point(minPoint);
			point[0] += stride[0] * x;
			point[1] += stride[1] * y;
			newAABB->update(point);
			point[0] += stride[0];
			point[1] += stride[1];
			point[2] += stride[2];
			newAABB->update(point);
			auto* procVoxel = new ProceduralVoxel(clouds[0], newAABB);
			subdivisions[x][y] = procVoxel;
		}
	}
}

/**
 * @brief Assign each point of the PointCloud to the corresponding voxel in the voxel grid
*/
void ProceduralGenerator::subdivideCloud() {
	std::cout << "Subdividing cloud..." << std::endl;
	unsigned pointCloudSize = clouds[0]->getNumberOfPoints();
	const std::vector<PointModel> points = clouds[0]->getPoints();

	glm::vec3 size = aabb.size();
	const glm::vec3 minPoint = aabb.min();
	float stride[2];

	for (unsigned i = 0; i < 2; i++) {
		stride[i] = size[i] / axisSubdivision[i];
	}

	#pragma omp parallel for
	for (int i = 0; i < points.size(); i++) {
		glm::vec3 relativePoint = points[i]._point - minPoint;
		unsigned x = floor(relativePoint[0] / stride[0]);
		unsigned y = floor(relativePoint[1] / stride[1]);
		if (x == axisSubdivision[0])
			x--;
		if (y == axisSubdivision[1])
			y--;
		#pragma omp critical
		subdivisions[x][y]->addPoint(i);
	}

	std::cout << "Computing height..." << std::endl;
	#pragma omp parallel for collapse(2)
	for (int x = 0; x < axisSubdivision[0]; x++) {
		for (int y = 0; y < axisSubdivision[1]; y++) {
			subdivisions[x][y]->computeHeight();
			subdivisions[x][y]->computeColor();
		}
	}

	saveHeightMap();
	saveTextureMap();
}

/**
 * @brief Saves the current voxel grid as a png file in gray scale that represents a height map
*/
void ProceduralGenerator::saveHeightMap() const {
	const float minPointZ = aabb.min()[2];
	const float relativeMaxPointZ = aabb.max()[2] - minPointZ;
	float relativeHeightValue;
	const auto pixels = new std::vector<unsigned char>();
	int i = 0;
	int o = 0;
	for (int y = 0; y < axisSubdivision[1]; y++) {
		for (int x = 0; x < axisSubdivision[0]; x++) {
			unsigned char color;
			const float height = subdivisions[x][y]->getHeight();
			if (height != FLT_MAX) {
				relativeHeightValue = (height - minPointZ) / relativeMaxPointZ;
			} else {
				relativeHeightValue = 0;
			}
			color = std::min(255, int(relativeHeightValue * 256.0f));
			glm::vec3 aux = subdivisions[x][y]->getColor();
			pixels->push_back(color);
			pixels->push_back(color);
			pixels->push_back(color);
			pixels->push_back(255);

		}
	}

	Image* image = new Image(pixels->data(), axisSubdivision[0], axisSubdivision[1], 4);
	//image->saveImage("heightmap.png");
}

/**
 * @brief Saves the current voxel grid as a png file in RGB scale that could be used as a texture of the terrain
*/
void ProceduralGenerator::saveTextureMap() {
	const float minPointZ = aabb.min()[2];
	float relativeMaxPointZ = aabb.max()[2] - minPointZ;
	float relativeHeightValue;
	const auto pixels = new std::vector<unsigned char>();
	for (int y = 0; y < axisSubdivision[1]; y++) {
		for (int x = 0; x < axisSubdivision[0]; x++) {
			glm::vec3 color = subdivisions[x][y]->getColor();
			pixels->push_back(color[0]);
			pixels->push_back(color[1]);
			pixels->push_back(color[2]);
			pixels->push_back(255);

		}
	}
	const auto image = new Image(pixels->data(), axisSubdivision[0], axisSubdivision[1], 4);
	//image->saveImage("texturemap.png");
}

void ProceduralGenerator::test() {
	std::cout << "Nurbs..." << std::endl;
	delete clouds[1];
	clouds[1] = new PPCX::PointCloud("DefaultSP");

	tinynurbs::Surface<float> srf;
	srf.degree_u = 5;
	srf.degree_v = 5;
	srf.knots_u.resize(axisSubdivision[0] + srf.degree_u + 1);
	srf.knots_v.resize(axisSubdivision[1] + srf.degree_v + 1);
	std::iota(srf.knots_u.begin(), srf.knots_u.end(), 0);
	std::iota(srf.knots_v.begin(), srf.knots_v.end(), 0);

	std::vector<glm::vec3> vec;
	for (int x = 0; x < axisSubdivision[0]; x++) {
		for (int y = 0; y < axisSubdivision[1]; y++) {
			glm::vec3 aux = subdivisions[x][y]->getMidPoint();
			if (aux[2] == FLT_MAX)
				meanHeight(x, y);
			aux = subdivisions[x][y]->getMidPoint();
			vec.push_back(aux);
		}
	}

	srf.control_points = { axisSubdivision[0], axisSubdivision[1], vec };
	if (tinynurbs::surfaceIsValid(srf)) {
		glm::vec3 minPoint = aabb.min();
		glm::vec3 maxPoint = aabb.max();
		PointModel point;
		#pragma omp parallel for private(point)
		for (int x = 0; x < axisSubdivision[0] * 5; x++) {
			for (int y = 0; y < axisSubdivision[1] * 5; y++) {
				point._point = tinynurbs::surfacePoint(srf, x * 0.2f, y * 0.2f);
				point.saveRGB(subdivisions[floor(x / 5)][floor(y / 5)]->getColor());
				#pragma omp critical
				clouds[1]->nuevoPunto(point);
			}
		}

		clouds[1]->actualizarNube();
	}
}


