#include "stdafx.h"
#include "ProceduralGenerator.h"

#include <filesystem>

#include "tinynurbs.h"
#include "Utilities/Image.h"
#include <random>

#include "Utilities/Loader.h"
#include <RendererCore/ModelManager.h>

#include <pcl/point_types.h>
#include <pcl/search/search.h>
#include <pcl/search/kdtree.h>
#include <pcl/segmentation/region_growing_rgb.h>

#include "RendererCore/InstancedPointCloud.h"
#include "RendererCore/Renderer.h"

std::vector<std::string> ProceduralGenerator::generatedCloudsName = { "Nurbs terrain cloud", "OliveTree.ply", "PineTree.ply" };

ProceduralGenerator::ProceduralGenerator() = default;

ProceduralGenerator::~ProceduralGenerator() {
	for (size_t x = 0; x < axisSubdivision[0]; x++) {
		for (size_t y = 0; y < axisSubdivision[1]; y++) {
			delete voxelGrid[x][y];
		}
	}
}


/**
 * @brief Initialize the voxel grid
*/
void ProceduralGenerator::createVoxelGrid(std::vector<std::vector<ProceduralVoxel*>>&grid, PointCloud * pointCloud) {
	std::cout << "Creating voxel grid..." << std::endl;
	this->progress = 0.2f;
	vec3 size = pointCloud->getAABB().size();
	const vec3 minPoint = pointCloud->getAABB().min();
	float stride[2];

	for (unsigned i = 0; i < 2; i++) {
		stride[i] = size[i] / axisSubdivision[i];
	}

	grid.resize(axisSubdivision[0]);
	for (size_t x = 0; x < axisSubdivision[0]; x++) {
		grid[x].resize(axisSubdivision[1]);
		for (size_t y = 0; y < axisSubdivision[1]; y++) {
			const auto newAABB = new AABB;
			vec3 point(minPoint);
			point[0] += stride[0] * x;
			point[1] += stride[1] * y;
			newAABB->update(point);
			point[0] += stride[0];
			point[1] += stride[1];
			newAABB->update(point);
			auto* procVoxel = new ProceduralVoxel(pointCloud, newAABB);
			grid[x][y] = procVoxel;
		}
	}
}

/**
 * @brief Assign each point of the PointCloud to the corresponding voxel in the voxel grid and computes the corresponding height and color
*/
void ProceduralGenerator::subdivideCloud(const std::vector<std::vector<ProceduralVoxel*>>&grid, PointCloud * pointCloud) {
	std::cout << "Subdividing cloud..." << std::endl;
	this->progress = 0.4f;
	const std::vector<PointModel>& points = pointCloud->getPoints();

	vec3 size = pointCloud->getAABB().size();
	const vec3 minPoint = pointCloud->getAABB().min();
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
		grid[x][y]->addPoint(i);
	}


}

void ProceduralGenerator::computeHeightAndColor() {

	std::cout << "Computing height and color..." << std::endl;
	this->progress = 0.5f;
	#pragma omp parallel for collapse(2)
	for (int x = 0; x < axisSubdivision[0]; x++) {
		for (int y = 0; y < axisSubdivision[1]; y++) {
			voxelGrid[x][y]->computeHeight();
			voxelGrid[x][y]->computeColor();
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

float ProceduralGenerator::getHeight(const glm::vec2 pos) const {
	vec3 size = aabb.size();
	const vec3 minPoint = aabb.min();
	float stride[2];

	for (unsigned i = 0; i < 2; i++) {
		stride[i] = size[i] / axisSubdivision[i];
	}

	const vec2 relativePoint = pos - glm::vec2(minPoint);
	int x = floor(relativePoint.x / stride[0]);
	int y = floor(relativePoint.y / stride[1]);
	if (x == axisSubdivision[0])
		x--;
	if (y == axisSubdivision[1])
		y--;

	return voxelGrid[x][y]->getHeight();
}

float ProceduralGenerator::getDensity(const glm::vec2 pos) const {
	vec3 size = aabb.size();
	const vec3 minPoint = aabb.min();
	float stride[2];

	for (unsigned i = 0; i < 2; i++) {
		stride[i] = size[i] / axisSubdivision[i];
	}

	const vec2 relativePoint = pos - glm::vec2(minPoint);
	int x = floor(relativePoint.x / stride[0]);
	int y = floor(relativePoint.y / stride[1]);
	if (x >= axisSubdivision[0])
		x = axisSubdivision[0] - 1;
	if (y >= axisSubdivision[1])
		y = axisSubdivision[1] - 1;

	return voxelGrid[x][y]->getNumberOfPoints() / (stride[0] * stride[1]);
}

void ProceduralGenerator::testRGBSegmentation() {

	const pcl::PointCloud <pcl::PointXYZRGB>::Ptr cloud(new pcl::PointCloud <pcl::PointXYZRGB>);
	const auto& points = dynamic_cast<PointCloud*>(ModelManager::getInstance()->getModel("High vegetation"))->getPoints();
	for (auto point : points) {
		const glm::vec3 color = point.getRGBVec3();
		cloud->emplace_back(pcl::PointXYZRGB(point._point.x, point._point.y, point._point.z, color.r, color.g, color.b));
	}
	#pragma omp parallel for
	for (int distance = 15; distance > 0; --distance) {
		for (int clusterSize = 5; clusterSize > 0; --clusterSize) {
			for (int regionColor = 0; regionColor < 6; ++regionColor) {
				for (int color = 8; color > 0; --color) {
					RegionRGBSegmentationUsingCloud(cloud, distance * 10, color, regionColor, clusterSize * 500);
				}
			}
		}
	}
}


//======================== Procedural methods ======================================


/**
 * Method to generate the NURBS based point cloud for the terrain.
 * The purpose of this method is to increase the ground cloud definition while maintaining this cloud characteristics (Curvatures, points colors...)
 *
 * @param degree NURBS degree
 * @param divX number of X subdivisions used by statitics method
 * @param divY number of Y subdivisions used by statitics method
 * @param desiredDensityMultiplier Density multiplier used to generate the NURBS point cloud. Using a value of 1, you will generate points only in voxels that don't have enought points (Empty voxels or voxels with big holes for example)
 */
void ProceduralGenerator::computeNURBS(unsigned degree, unsigned divX, unsigned divY, float desiredDensityMultiplier) {
	std::cout << "Creating nurbs..." << std::endl;
	this->progress = 0.6f;

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
	unsigned numPoints = terrainCloud->getNumberOfPoints();
	float density;
	for (int x = 0; x < axisSubdivision[0]; x++) {
		for (int y = 0; y < axisSubdivision[1]; y++) {
			vec3 aux = voxelGrid[x][y]->getRepresentativePoint();
			controlPoints.push_back(aux);
			density = static_cast<float>(voxelGrid[x][y]->getNumberOfPoints() + 1);
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
				unsigned limit = voxelGrid[x][y]->numberPointsToDensity(cloudDensity * desiredDensityMultiplier);
				std::vector<float> intervals[2];
				std::vector<float> weights[2];
				if (voxelGrid[x][y]->getNumberOfPoints() > cloudDensity * 0.2f && useStatiticsMethod) {
					std::vector<float> allWeights = voxelGrid[x][y]->internalDistribution(divX, divY);
					++count;
					for (unsigned i = 0; i < divX; i++) {
						intervals[0].push_back(x + static_cast<float>(i) / divX);
						float sum = 0;
						for (unsigned j = 0; j < divY; j++) {
							sum += allWeights[i * divX + j];
						}
						weights[0].push_back(sum);
					}
					intervals[0].push_back(x + 1);
					for (unsigned i = 0; i < divY; i++) {
						intervals[1].push_back(y + static_cast<float>(i) / divY);
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
					if (voxelGrid[x][y]->getNumberOfPoints() > cloudDensity * 0.2f && useStatiticsMethod) {
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
					vec3 color = voxelGrid[posColorX][posColorY]->getColor();
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

		ModelManager::getInstance()->modifyModel("Nurbs terrain cloud", new PointCloud("DefaultSP", points, newAABB));
		this->progress = FLT_MAX;
	}
}


void ProceduralGenerator::RegionRGBSegmentation(const float distanceThreshold, const float pointColorThreshold, const float regionColorThreshold, const unsigned minClusterSize) {
	const pcl::search::Search <pcl::PointXYZRGB>::Ptr tree(new pcl::search::KdTree<pcl::PointXYZRGB>);
	const pcl::PointCloud <pcl::PointXYZRGB>::Ptr cloud(new pcl::PointCloud <pcl::PointXYZRGB>);

	this->progress = 1.15f;

	const auto& points = dynamic_cast<PointCloud*>(ModelManager::getInstance()->getModel("High vegetation"))->getPoints();
	std::string filename = "dist-" + std::to_string(distanceThreshold);
	filename.append("color-" + std::to_string(pointColorThreshold));
	filename.append("regionColor-" + std::to_string(regionColorThreshold));
	filename.append("minClusterSize-" + std::to_string(minClusterSize));
	filename.append("-" + std::to_string(points.size()));
	if (!this->loadClusterBinary(filename)) {
		for (auto point : points) {
			const glm::vec3 color = point.getRGBVec3();
			cloud->emplace_back(pcl::PointXYZRGB(point._point.x, point._point.y, point._point.z, color.r, color.g, color.b));
		}

		pcl::RegionGrowingRGB<pcl::PointXYZRGB> reg;
		reg.setInputCloud(cloud);
		reg.setSearchMethod(tree);
		reg.setDistanceThreshold(distanceThreshold);
		reg.setPointColorThreshold(pointColorThreshold);
		reg.setRegionColorThreshold(regionColorThreshold);
		reg.setMinClusterSize(minClusterSize);

		this->progress = 1.3f;
		std::vector <pcl::PointIndices> clusters;
		reg.extract(clusters);

		this->progress = 1.8f;

		PointModel point;
		std::vector<PointCloud*> clouds;
		for (unsigned i = 0; i < clusters.size(); i++) {
			const auto newCloud = new PointCloud("DefaultSP");
			glm::vec3 color(rand() % 256, rand() % 256, rand() % 256);
			for (const auto& origPoint : clusters[i].indices) {
				point._point.x = (*cloud)[origPoint].x;
				point._point.y = (*cloud)[origPoint].y;
				point._point.z = (*cloud)[origPoint].z;

				point.saveRGB(color);
				newCloud->newPoint(point);
			}
			clouds.push_back(newCloud);
			ModelManager::getInstance()->modifyModel("RGB Region Segment " + std::to_string(i), newCloud);
			generatedCloudsName.emplace_back("RGB Region Segment " + i);
		}
		this->saveClusterBinary(filename, clouds);
	}

	this->progress = FLT_MAX;
}

void ProceduralGenerator::RegionRGBSegmentationUsingCloud(const pcl::PointCloud<pcl::PointXYZRGB>::Ptr & cloud, const float distanceThreshold, const float pointColorThreshold, const float regionColorThreshold, const unsigned minClusterSize) {
	static unsigned currentRegion = 0;

	std::string filename = "dist-" + std::to_string(distanceThreshold);
	filename.append("   color-" + std::to_string(pointColorThreshold));
	filename.append("   regionColor-" + std::to_string(regionColorThreshold));
	filename.append("   minClusterSize-" + std::to_string(minClusterSize));
	if (FILE* file = fopen(("Captures/" + filename + ".png").c_str(), "r")) {
		fclose(file);
		std::cout << filename << " ya existe" << std::endl;
		return;
	}
	std::cout << filename << std::endl;

	const pcl::search::Search <pcl::PointXYZRGB>::Ptr tree(new pcl::search::KdTree<pcl::PointXYZRGB>);

	pcl::RegionGrowingRGB<pcl::PointXYZRGB> reg;
	reg.setInputCloud(cloud);
	reg.setSearchMethod(tree);
	reg.setDistanceThreshold(distanceThreshold);
	reg.setPointColorThreshold(pointColorThreshold);
	reg.setRegionColorThreshold(regionColorThreshold);
	reg.setMinClusterSize(minClusterSize);

	std::vector <pcl::PointIndices> clusters;
	reg.extract(clusters);


	const pcl::PointCloud <pcl::PointXYZRGB>::Ptr colored_cloud = reg.getColoredCloud();

	const auto newCloud = new PointCloud("DefaultSP");
	PointModel point;
	for (std::size_t i = 0; i < colored_cloud->size(); ++i) {
		point._point.x = (*colored_cloud)[i].x;
		point._point.y = (*colored_cloud)[i].y;
		point._point.z = (*colored_cloud)[i].z;

		point.saveRGB(glm::vec3((*colored_cloud)[i].r, (*colored_cloud)[i].g, (*colored_cloud)[i].b));
		newCloud->newPoint(point);
	}
	ModelManager::getInstance()->newModel("RGB Region Segmentation" + currentRegion, newCloud);

	#pragma omp critical
	PPCX::Renderer::getInstancia()->addPendingScreenshot(filename, "RGB Region Segmentation" + currentRegion++);

	this->progress = FLT_MAX;
}

void ProceduralGenerator::generateProceduralVegetation(const std::vector<std::pair<std::string, std::string>>&data) {
	int c = 0;
	for (auto& pair : data) {
		std::cout << pair.first << std::endl;
		std::vector<std::vector<ProceduralVoxel*>> grid;
		const auto& cloud = dynamic_cast<PointCloud*>(ModelManager::getInstance()->getModel(pair.first));
		const auto& instancedModel = dynamic_cast<InstancedPointCloud*>(ModelManager::getInstance()->getModel(pair.second));
		const glm::vec3 treeSize = instancedModel->getAABB().size();

		createVoxelGrid(grid, cloud);
		subdivideCloud(grid, cloud);
		for (size_t i = 0; i < grid.size(); i++) {
			for (size_t j = 0; j < grid[i].size(); j++) {
				vec3 center = grid[i][j]->getCenter();
				if (grid[i][j]->getVegetationMark())
					c++;
				if (grid[i][j]->getDensity() > getDensity(center) * 2.0f && !grid[i][j]->getVegetationMark()) {
					vec3 pos = center;
					pos.z = getHeight(center);
					instancedModel->newInstance(pos, { 0, 0,rand()%360 }, { 1,1,1 });
					const glm::vec3 size = grid[i][j]->getAABB().size();
					const float x = treeSize.x / size.x;
					const float y = treeSize.y / size.y;
					int startX = i, endX = i, startY = j, endY = j;
					if (x > 0) {
						endX = i + (x / 2 + 0.5);
						if (endX >= grid[i].size())
							endX = grid[i].size() - 1;
					}
					if (y > 0) {
						endY = j + (y / 2 + 0.5);
						if (endY >= grid[i].size())
							endY = grid[i].size() - 1;
					}
					for (int k = startX; k <= endX; ++k) {
						for (int l = startY; l <= endY; ++l) {
							grid[k][l]->setVegetationMark();
						}
					}
					/*std::cout << "Pos i:" << i << " - Pos j: " << j << " -> " << treeSize.x << "-" << size.x << std::endl;
					std::cout << "X: " << startX << "-" << endX << std::endl << "Y: " << startY << "-" << endY << std::endl << std::endl;*/
				}
			}
		}
		for (auto& line : grid) {
			for (const auto& voxel : line) {
				delete voxel;
			}
		}

	}

	std::cout << "Q: " << c << std::endl;
	this->progress = FLT_MAX;
}



//======================== Auxiliar methods ======================================


/**
 * Delete the current voxel grid and generate a new one using the pointsPerVoxel parameter
 *
 * @param pointsPerVoxel see automaticGSD method.
 */
void ProceduralGenerator::generateVoxelGrid(const unsigned pointsPerVoxel) {
	for (size_t x = 0; x < axisSubdivision[0]; x++) {
		for (size_t y = 0; y < axisSubdivision[1]; y++) {
			delete voxelGrid[x][y];
		}
	}

	automaticGSD(pointsPerVoxel);
	createVoxelGrid(voxelGrid, terrainCloud);
	subdivideCloud(voxelGrid, terrainCloud);
	computeHeightAndColor();
	this->progress = FLT_MAX;
}


/**
* @brief Compute the voxel grid appropiate size automatically.
* @param pointsPerVoxel number of points that should be in each voxel. Less points = more and smallers voxels
*/
void ProceduralGenerator::automaticGSD(const unsigned pointsPerVoxel) {
	const unsigned voxelsNumber = terrainCloud->getNumberOfPoints() / pointsPerVoxel;
	const vec3 cloudSize = terrainCloud->getAABB().size();
	const float sizeProportion = cloudSize.x / cloudSize.y;
	axisSubdivision[1] = sqrt(voxelsNumber / sizeProportion);
	axisSubdivision[0] = sizeProportion * axisSubdivision[1];

}


/**
 * Method that calculates the height and the color of a voxel based on the height and color of the neightbours
 *
 * @param x x subdivision of the voxel
 * @param y y subdivision of the voxel
 * @param minCount number of valid neighbours needed to set the calculated height and color.
 * @return true if the values are setted. False otherwise.
 */
bool ProceduralGenerator::meanNeightbourHeightColor(const unsigned x, const unsigned y, const char minCount) const {
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
				const vec3 color = voxelGrid[auxX][auxY]->getColor();
				const float height = voxelGrid[auxX][auxY]->getHeight();
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
		voxelGrid[x][y]->setHeight(heightMean);
		voxelGrid[x][y]->setColor(colorMean);
		return true;
	}
	return false;
}


void ProceduralGenerator::newPointCloud(PointCloud * pCloud, const bool newScene, const unsigned pointsPerVoxel) {
	if (newScene || !terrainCloud) {
		terrainCloud = pCloud;
		cloudDensity = terrainCloud->getDensity();
	} else {
		const auto& vec = pCloud->getPoints();
		for (auto& i : vec) {
			terrainCloud->newPoint(i);
		}
		terrainCloud->needUpdating = true;
	}
	this->aabb = terrainCloud->getAABB();
	generateVoxelGrid(pointsPerVoxel);
}


//======================== Save methods ======================================


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
			const float height = voxelGrid[x][y]->getHeight();
			if (height != FLT_MAX) {
				relativeHeightValue = (height - minPointZ) / relativeMaxPointZ;
			} else {
				relativeHeightValue = 0;
			}
			color = std::min(255, static_cast<int>(relativeHeightValue * 256.0f));
			vec3 aux = voxelGrid[x][y]->getColor();
			pixels->push_back(color);
			pixels->push_back(color);
			pixels->push_back(color);
			pixels->push_back(255);
		}
	}

	const auto image = new Image(pixels->data(), axisSubdivision[0], axisSubdivision[1], 4);
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
			vec3 color = voxelGrid[x][y]->getColor();
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
	//aux.push_back(terrainCloud[0]);
	//aux.push_back(terrainCloud[1]);
	std::thread thread(&Loader::savePointCloud, path, aux);
	thread.detach();
}

void ProceduralGenerator::saveClusterBinary(const std::string & filename, const std::vector<PointCloud*>&clouds) {
	std::ofstream wf(filename + ".bin", std::ios::out | std::ios::binary);
	if (!wf.is_open()) {
		throw std::runtime_error("Error while writing binary cluster file");
	}
	for (const auto& cloud : clouds) {
		const size_t numPoints = cloud->getNumberOfPoints();
		wf.write((char*)&numPoints, sizeof(size_t));
		wf.write((char*)&cloud->getPoints()[0], numPoints * sizeof(PointModel));
		wf.write((char*)&cloud->getAABB(), sizeof(AABB));

	}
	wf.close();
}

bool ProceduralGenerator::loadClusterBinary(const std::string & filename) const {
	if (std::filesystem::exists(filename + ".bin")) {
		std::ifstream rf(filename + ".bin", std::ios::in | std::ios::binary);
		if (!rf.is_open()) {
			throw std::runtime_error("Error while reading binary cluster file");
		}
		unsigned i = 0;
		while (!rf.eof()) {
			size_t numPoints;
			std::vector<PointModel> points;
			AABB _aabb;

			rf.read((char*)&numPoints, sizeof(size_t));
			points.resize(numPoints);
			rf.read((char*)&points[0], numPoints * sizeof(PointModel));
			rf.read((char*)&_aabb, sizeof(AABB));

			PointCloud* cloud = new PointCloud("DefaultSP", points, aabb);
			ModelManager::getInstance()->newModel("RGB Region Segment" + std::to_string(i), cloud);
			generatedCloudsName.emplace_back("RGB Region Segment " + i);
			i++;
		}
		rf.close();
		return true;
	}
	return false;
}
