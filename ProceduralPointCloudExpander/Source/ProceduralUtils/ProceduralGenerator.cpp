#include "stdafx.h"
#include "ProceduralGenerator.h"
#include "tinynurbs.h"
#include "Utilities/Image.h"
#include <random>

#include "Utilities/PlyLoader.h"

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

void ProceduralGenerator::drawClouds(const mat4 matrizMVP) {
	for (PointCloud*& cloud : clouds) {
		if (cloud) {
			cloud->drawModel(matrizMVP);
		}
	}
}

void ProceduralGenerator::newPointCloud(PointCloud * pCloud, bool newScene, unsigned pointsPerVoxel) {
	if (newScene || !clouds[0]) {
		for (const auto& cloud : clouds) {
			delete cloud;
		}
		clouds[0] = pCloud;
		cloudDensity = clouds[0]->getDensity();
	} else {
		const auto& vec = pCloud->getPoints();
		for (auto& i : vec) {
			clouds[0]->newPoint(i);
		}
		clouds[0]->needUpdating = true;
	}
	this->aabb = clouds[0]->getAABB();
	generateVoxelGrid(pointsPerVoxel);
}


bool ProceduralGenerator::meanNeightbourHeightColor(unsigned x, unsigned y, char minCount) const {
	float heightMean = 0;
	vec3 colorMean = { 0,0,0 };
	char counter = 0;
	for (int i = -1; i < 2; i++) {
		for (int j = -1; j < 2; j++) {
			int auxX = x + i;
			auxX = std::min(static_cast<int>(axisSubdivision[0]) - 1, std::max(0, auxX));
			int auxY = y + j;
			auxY = std::min(static_cast<int>(axisSubdivision[1]) - 1, std::max(0, auxY));
			if (auxX != x || auxY != y) {
				const vec3 color = subdivisions[auxX][auxY]->getColor();
				const float height = subdivisions[auxX][auxY]->getHeight();
				if (height != FLT_MAX) {
					heightMean += height;
					colorMean += color;
					counter++;
				}
			}
		}
	}
	if (counter >= minCount) {
		heightMean /= counter;
		colorMean /= counter;
		subdivisions[x][y]->setHeight(heightMean);
		subdivisions[x][y]->setColor(colorMean);
		return true;
	}
	return false;
}


/**
 * @brief Initialize the voxel grid
*/
void ProceduralGenerator::createVoxelGrid() {
	std::cout << "Creating voxel grid..." << std::endl;
	this->progress = 0.2f;
	vec3 size = aabb.size();
	const vec3 minPoint = aabb.min();
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
			vec3 point(minPoint);
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

	vec3 size = aabb.size();
	const vec3 minPoint = aabb.min();
	float stride[2];

	for (unsigned i = 0; i < 2; i++) {
		stride[i] = size[i] / axisSubdivision[i];
	}

	#pragma omp parallel for
	for (int i = 0; i < points.size(); i++) {
		const vec3 relativePoint = points[i]._point - minPoint;
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
	std::list<std::pair<unsigned, unsigned>> unfinishedVoxels;
	for (int x = 0; x < axisSubdivision[0]; x++) {
		for (int y = 0; y < axisSubdivision[1]; y++) {
			if (!meanNeightbourHeightColor(x, y, 8)) {
				auto pair = std::make_pair(x, y);
				unfinishedVoxels.emplace_back(pair);
			}
		}
	}

	for (int i = 7; i >= 1; i--) {
		for (auto it = unfinishedVoxels.begin(); it != unfinishedVoxels.end(); ++it) {
			if (meanNeightbourHeightColor(it->first, it->second, i)) {
				it = unfinishedVoxels.erase(it);
				if (it == unfinishedVoxels.end())
					break;
			}
		}
	}
	while (!unfinishedVoxels.empty()) {
		for (auto it = unfinishedVoxels.begin(); it != unfinishedVoxels.end(); ++it) {
			if (meanNeightbourHeightColor(it->first, it->second, 1)) {
				it = unfinishedVoxels.erase(it);
				if (it == unfinishedVoxels.end())
					break;
			}
		}
	}

}

void ProceduralGenerator::computeNURBS(unsigned degree, unsigned divX, unsigned divY, float desiredDensityMultiplier) {
	std::cout << "Creating nurbs..." << std::endl;
	this->progress = 0.6f;
	delete clouds[1];
	clouds[1] = nullptr;

	tinynurbs::RationalSurface<float> srf;
	srf.degree_u = degree;
	srf.degree_v = degree;
	srf.knots_u.resize(axisSubdivision[0] + degree + 1);
	srf.knots_v.resize(axisSubdivision[1] + degree + 1);

	for (unsigned i = 0; i < degree + 1; i++) {
		srf.knots_u[i] = 0;

	}
	for (unsigned i = 0; i < degree + 1; i++) {
		srf.knots_v[i] = 0;
	}
	std::iota(srf.knots_u.begin() + degree + 1, srf.knots_u.end() - (degree + 1), 1);
	std::iota(srf.knots_v.begin() + degree + 1, srf.knots_v.end() - (degree + 1), 1);
	for (unsigned i = axisSubdivision[0]; i < srf.knots_u.size(); i++) {
		srf.knots_u[i] = axisSubdivision[0] - degree;

	}
	for (unsigned i = axisSubdivision[1]; i < srf.knots_v.size(); i++) {
		srf.knots_v[i] = axisSubdivision[1] - degree;
	}

	std::vector<vec3> controlPoints;
	std::vector<float> weights;
	unsigned numPoints = clouds[0]->getNumberOfPoints();
	float density;
	for (int x = 0; x < axisSubdivision[0]; x++) {
		for (int y = 0; y < axisSubdivision[1]; y++) {
			vec3 aux = subdivisions[x][y]->getRepresentativePoint();
			controlPoints.push_back(aux);
			density = static_cast<float>(subdivisions[x][y]->getNumberOfPoints() + 1);
			weights.push_back(pow(density, 2));
		}
	}

	srf.control_points = { axisSubdivision[0], axisSubdivision[1], controlPoints };
	srf.weights = { axisSubdivision[0], axisSubdivision[1], weights };

	if (surfaceIsValid(srf)) {
		std::cout << "Generating nurbs cloud..." << std::endl;
		this->progress = .70f;

		tinynurbs::array2<vec<4, float>> Cw;
		Cw.resize(srf.control_points.rows(), srf.control_points.cols());
		for (int i = 0; i < srf.control_points.rows(); i++) {
			for (int j = 0; j < srf.control_points.cols(); j++) {
				Cw(i, j) = vec(tinynurbs::util::cartesianToHomogenous(srf.control_points(i, j), srf.weights(i, j)));
			}
		}

		vec3 minPoint = aabb.min();
		vec3 maxPoint = aabb.max();
		PointModel point;
		std::vector<PointModel> points;
		AABB newAABB;

		unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
		std::cout << "[DEBUG]: Seed-" << seed << std::endl;
		std::default_random_engine generator(seed);
		std::uniform_int_distribution<unsigned> genColor(0, 20);
		std::atomic<int> count = 0;
		#pragma omp parallel for private(point, generator)
		for (int x = 0; x < axisSubdivision[0]; x++) {
			for (int y = 0; y < axisSubdivision[1]; y++) {
				unsigned limit = subdivisions[x][y]->numberPointsToDensity(cloudDensity * desiredDensityMultiplier);
				std::vector<float> intervals[2];
				std::vector<float> weights[2];
				if (subdivisions[x][y]->getNumberOfPoints() > cloudDensity * 0.3f && useStatiticsMethod) {
					std::vector<float> allWeights = subdivisions[x][y]->internalDistribution(divX, divY);
					++count;
					for (unsigned i = 0; i < divX; i++) {
						intervals[0].push_back(x + (float)i / divX);
						float sum = 0;
						for (unsigned j = 0; j < divY; j++) {
							sum += allWeights[i * divX + j];
						}
						weights[0].push_back(sum);
					}
					intervals[0].push_back(x + 1);
					for (unsigned i = 0; i < divY; i++) {
						intervals[1].push_back(y + (float)i / divY);
						float sum = 0;
						for (unsigned j = 0; j < divX; j++) {
							sum += allWeights[i * divY + j];
						}
						weights[1].push_back(sum);
					}
					intervals[1].push_back(y + 1);
				}
				std::uniform_real_distribution<float> disX(x, x + 1);
				std::uniform_real_distribution<float> disY(y, y + 1);
				std::piecewise_constant_distribution<float> customDisX(intervals[0].begin(), intervals[0].end(), weights[0].begin());
				std::piecewise_constant_distribution<float> customDisY(intervals[1].begin(), intervals[1].end(), weights[1].begin());
				for (int i = 0; i < limit; i++) {
					float valX;
					float valY;
					if (subdivisions[x][y]->getNumberOfPoints() > cloudDensity * 0.3f && useStatiticsMethod) {
						valX = customDisX(generator);
						valY = customDisY(generator);
						//std::cout << "x: " << x << " y: " << y << " valX: " << valX << " valY: " << valY << std::endl;
					} else {
						valX = disX(generator);
						valY = disY(generator);
					}
					point._point = surfacePoint(srf, valX, valY, Cw);
					int posColorX = valX + 0.5f;
					int posColorY = valY + 0.5f;
					if (posColorX >= axisSubdivision[0])
						posColorX = axisSubdivision[0] - 1;
					if (posColorY >= axisSubdivision[1])
						posColorY = axisSubdivision[1] - 1;
					vec3 color = subdivisions[posColorX][posColorY]->getColor();
					color.r += genColor(generator);
					color.g += genColor(generator);
					color.b += genColor(generator);
					point.saveRGB(color);
					if (point._point.z < maxPoint.z + 20 && point._point.z > minPoint.z - 20) {
						#pragma omp critical
						points.push_back(point);
						#pragma omp critical
						newAABB.update(point._point);
					}
				}
			}
		}
		std::cout << count << std::endl;
		clouds[1] = new PointCloud("DefaultSP", points, newAABB);
		this->progress = 2.0f;
	}
}

void ProceduralGenerator::generateVoxelGrid(const unsigned pointsPerVoxel) {
	for (size_t x = 0; x < axisSubdivision[0]; x++) {
		for (size_t y = 0; y < axisSubdivision[1]; y++) {
			delete subdivisions[x][y];
		}
	}
	delete clouds[1];
	clouds[1] = nullptr;
	automaticGSD(pointsPerVoxel);
	createVoxelGrid();
	subdivideCloud();
	this->progress = 2.0f;
}

/**
* @brief Compute the voxel grid appropiate size automatically.
* @param pointsPerVoxel number of points that should be in each voxel. Less points = more and smallers voxels
*/
void ProceduralGenerator::automaticGSD(unsigned pointsPerVoxel) {
	const unsigned voxelsNumber = clouds[0]->getNumberOfPoints() / pointsPerVoxel;
	const vec3 cloudSize = clouds[0]->getAABB().size();
	const float sizeProportion = cloudSize.x / cloudSize.y;
	axisSubdivision[1] = sqrt(voxelsNumber / sizeProportion);
	axisSubdivision[0] = sizeProportion * axisSubdivision[1];

}



/**
 * @brief Saves the current voxel grid as a png file in gray scale that represents a height map
*/
void ProceduralGenerator::saveHeightMap(const std::string & path) const {
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
			color = std::min(255, static_cast<int>(relativeHeightValue * 256.0f));
			vec3 aux = subdivisions[x][y]->getColor();
			pixels->push_back(color);
			pixels->push_back(color);
			pixels->push_back(color);
			pixels->push_back(255);
		}
	}

	auto image = new Image(pixels->data(), axisSubdivision[0], axisSubdivision[1], 4);
	image->saveImage(path);
}

/**
 * @brief Saves the current voxel grid as a png file in RGB scale that could be used as a texture of the terrain
*/
void ProceduralGenerator::saveTextureMap(const std::string & path) const {
	const float minPointZ = aabb.min()[2];
	float relativeMaxPointZ = aabb.max()[2] - minPointZ;
	float relativeHeightValue = 0;
	const auto pixels = new std::vector<unsigned char>();
	for (int y = 0; y < axisSubdivision[1]; y++) {
		for (int x = 0; x < axisSubdivision[0]; x++) {
			vec3 color = subdivisions[x][y]->getColor();
			pixels->push_back(color[0]);
			pixels->push_back(color[1]);
			pixels->push_back(color[2]);
			pixels->push_back(255);

		}
	}
	const auto image = new Image(pixels->data(), axisSubdivision[0], axisSubdivision[1], 4);
	image->saveImage(path);
}

/*
* @brief Saves the current point cloud as a PLY file.
*/
void ProceduralGenerator::savePointCloud(const std::string & path) const {
	std::vector<PointCloud*> aux;
	aux.push_back(clouds[0]);
	aux.push_back(clouds[1]);
	std::thread thread(&PlyLoader::savePointCloud, path, aux);
	thread.detach();
}


bool& ProceduralGenerator::getPointCloudVisibility(unsigned cloud) const {
	if (clouds[cloud])
		return clouds[cloud]->getVisible();
	return clouds[0]->getVisible();
}