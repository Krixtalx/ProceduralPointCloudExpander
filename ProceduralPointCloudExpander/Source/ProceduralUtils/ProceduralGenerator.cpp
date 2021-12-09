#include "stdafx.h"
#include "ProceduralGenerator.h"
#include "tinynurbs.h"
#include "Utilities/Image.h"
#include <random>

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
	for (PPCX::PointCloud*& cloud : clouds) {
		if (cloud) {
			if (cloud->needUpdating)
				cloud->actualizarNube();
			cloud->dibujarModelo(matrizMVP);
		}
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
		cloudDensity = clouds[0]->getDensity();
	} else {
		const auto vec = pCloud->getPoints();
		for (auto i : vec) {
			clouds[0]->nuevoPunto(i);
		}
		clouds[0]->needUpdating=true;
	}
	this->aabb = clouds[0]->getAABB();
	readParameters("proceduralParameters.ini");
	createVoxelGrid();
	subdivideCloud();
	computeNURBS();
}

bool& ProceduralGenerator::getPointCloudVisibility(unsigned cloud) {
	if (clouds[cloud])
		return clouds[cloud]->getVisible();
	else
		return clouds[0]->getVisible();
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
			axisSubdivision[i] = size[i] / gsd;
		}
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
 * @brief compute the height of a proceduralVoxel as the mean of the inmediate neightbour
 * @param x index of the subdivision
 * @param y index of the subdivision
*/
void ProceduralGenerator::meanColor(unsigned x, unsigned y) {
	glm::vec3 mean = { 0,0,0 };
	char counter = 0;
	for (int i = -1; i < 2; i++) {
		for (int j = -1; j < 2; j++) {
			int auxX = x + i;
			auxX = std::min((int)axisSubdivision[0] - 1, std::max(0, auxX));
			int auxY = y + j;
			auxY = std::min((int)axisSubdivision[1] - 1, std::max(0, auxY));
			if (auxX != x || auxY != y) {
				counter++;
				const glm::vec3 color = subdivisions[auxX][auxY]->getColor();
				if (subdivisions[auxX][auxY]->getHeight() != FLT_MAX)
					mean += color;
				else
					counter--;
			}
		}
	}
	if (counter > 0) {
		mean /= counter;
		subdivisions[x][y]->setColor(mean);
	}
}


/**
 * @brief Initialize the voxel grid
*/
void ProceduralGenerator::createVoxelGrid() {
	std::cout << "Creating voxel grid..." << std::endl;
	this->progress = 0.2f;
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
	this->progress = 0.4f;
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
		int x = floor(relativePoint.x / stride[0]);
		int y = floor(relativePoint.y / stride[1]);
		if (x == axisSubdivision[0])
			x--;
		if (y == axisSubdivision[1])
			y--;
		#pragma omp critical
		subdivisions[x][y]->addPoint(i);
	}

	std::cout << "Computing height..." << std::endl;
	this->progress = 0.5f;
	#pragma omp parallel for collapse(2)
	for (int x = 0; x < axisSubdivision[0]; x++) {
		for (int y = 0; y < axisSubdivision[1]; y++) {
			subdivisions[x][y]->computeHeight();
			subdivisions[x][y]->computeColor();
		}
	}

	//saveHeightMap();
	//saveTextureMap();
}

/**
 * @brief Saves the current voxel grid as a png file in gray scale that represents a height map
*/
void ProceduralGenerator::saveHeightMap(std::string path) const {
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
	image->saveImage(path);
}

/**
 * @brief Saves the current voxel grid as a png file in RGB scale that could be used as a texture of the terrain
*/
void ProceduralGenerator::saveTextureMap(std::string path) {
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
	image->saveImage(path);
}

void ProceduralGenerator::computeNURBS() {
	std::cout << "Creating nurbs..." << std::endl;
	this->progress = 0.6f;
	delete clouds[1];

	tinynurbs::RationalSurface<float> srf;
	int degree = 2;
	srf.degree_u = degree;
	srf.degree_v = degree;
	srf.knots_u.resize(axisSubdivision[0] + degree + 1);
	srf.knots_v.resize(axisSubdivision[1] + degree + 1);
	/*for (int i = 0; i < srf.knots_u.size(); i++) {
		srf.knots_u[i] = (int)i / (degree + 1);

	}
	for (int i = 0; i < srf.knots_v.size(); i++) {
		srf.knots_v[i] = (int)i / (degree + 1);
	}*/

	for (int i = 0; i < (degree + 1); i++) {
		srf.knots_u[i] = 0;

	}
	for (int i = 0; i < (degree + 1); i++) {
		srf.knots_v[i] = 0;
	}
	std::iota(srf.knots_u.begin() + degree + 1, srf.knots_u.end() - (degree + 1), 1);
	std::iota(srf.knots_v.begin() + degree + 1, srf.knots_v.end() - (degree + 1), 1);
	for (int i = axisSubdivision[0]; i < srf.knots_u.size(); i++) {
		srf.knots_u[i] = axisSubdivision[0] - degree;

	}
	for (int i = axisSubdivision[1]; i < srf.knots_v.size(); i++) {
		srf.knots_v[i] = axisSubdivision[1] - degree;
	}

	/*std::iota(srf.knots_u.begin(), srf.knots_u.end(), 0);
	std::iota(srf.knots_v.begin(), srf.knots_v.end(), 0);*/

	std::vector<glm::vec3> controlPoints;
	std::vector<float> weights;
	unsigned numPoints = clouds[0]->getNumberOfPoints();
	float density;
	for (int x = 0; x < axisSubdivision[0]; x++) {
		for (int y = 0; y < axisSubdivision[1]; y++) {
			glm::vec3 aux = subdivisions[x][y]->getRepresentativePoint();
			if (aux[2] == FLT_MAX) {
				meanHeight(x, y);
				meanColor(x, y);
			}
			aux = subdivisions[x][y]->getRepresentativePoint();
			controlPoints.push_back(aux);
			density = (float)(subdivisions[x][y]->getNumberOfPoints() + 1);
			weights.push_back(density);
		}
	}

	srf.control_points = { axisSubdivision[0], axisSubdivision[1], controlPoints };
	srf.weights = { axisSubdivision[0], axisSubdivision[1], weights };

	if (tinynurbs::surfaceIsValid(srf)) {
		std::cout << "Generating nurbs cloud..." << std::endl;
		this->progress = .75f;
		tinynurbs::array2<glm::vec<4, float>> Cw;
		Cw.resize(srf.control_points.rows(), srf.control_points.cols());
		for (int i = 0; i < srf.control_points.rows(); i++) {
			for (int j = 0; j < srf.control_points.cols(); j++) {
				Cw(i, j) =
					glm::vec<4, float>(tinynurbs::util::cartesianToHomogenous(srf.control_points(i, j), srf.weights(i, j)));
			}
		}

		glm::vec3 minPoint = aabb.min();
		glm::vec3 maxPoint = aabb.max();
		PointModel point;
		std::vector<PointModel> points;
		AABB newAABB;

		/*point._point = tinynurbs::surfacePoint(srf, .0f, .0f, Cw);
		std::cout << point._point.x << " - " << point._point.y << " - " << point._point.z << std::endl;
		std::cout << minPoint.x << " - " << minPoint.y << " - " << minPoint.z << std::endl;*/
		unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
		std::default_random_engine generator(seed);
		std::uniform_real_distribution<float> genColor(0, 0.05f);
		#pragma omp parallel for private(point, generator)
		for (int x = 0; x < axisSubdivision[0]; x++) {
			for (int y = 0; y < axisSubdivision[1]; y++) {
				unsigned limit = subdivisions[x][y]->numberPointsToDensity(cloudDensity * 2);
				std::uniform_real_distribution<float> disX(x, x + 1);
				std::uniform_real_distribution<float> disY(y, y + 1);
				for (int i = 0; i < limit; i++) {
					float valX = disX(generator);
					float valY = disY(generator);
					point._point = tinynurbs::surfacePoint(srf, valX, valY, Cw);
					/*point._point.x += (gsd / 2) * (degree);
					point._point.y += (gsd / 2) * (degree);*/
					/*std::cout << valX << "-" << valY << ": " << point._point.x << "-" << point._point.y << "-" << point._point.z << std::endl;*/
					int posColorX = valX + 0.5f;
					int posColorY = valY + 0.5f;
					if (posColorX >= axisSubdivision[0])
						posColorX = axisSubdivision[0] - 1;
					if (posColorY >= axisSubdivision[1])
						posColorY = axisSubdivision[1] - 1;
					glm::vec3 color = subdivisions[posColorX][posColorY]->getColor();
					color.r += genColor(generator);
					color.g += genColor(generator);
					color.b += genColor(generator);
					point.saveRGB(color);
					if (point._point.z < maxPoint.z + 100 && point._point.z > minPoint.z - 50) {
						#pragma omp critical
						points.push_back(point);
						#pragma omp critical
						newAABB.update(point._point);
					}
				}
			}
		}
		clouds[1] = new PPCX::PointCloud("DefaultSP", points, newAABB);
		clouds[1]->needUpdating = true;
		this->progress = 2.0f;
	}
}