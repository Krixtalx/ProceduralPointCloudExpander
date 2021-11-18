#include "stdafx.h"
#include "ProceduralGenerator.h"
#include <omp.h>
#include "tinynurbs.h"
#include "Utilities/Image.h"

/**
 * @brief Set the pointCloudScene passed by parameters as the current pointCloudScene of the generator
 * deleting all previous work(if any) and redoing all the process from the start
 * @param pointCloudScene New pointCloudScene
*/
void ProceduralGenerator::setCurrentCloudScene(PointCloudScene* pointCloudScene)
{
	for (size_t x = 0; x < axisSubdivision[0]; x++)
	{
		for (size_t y = 0; y < axisSubdivision[1]; y++)
		{
			for (size_t z = 0; z < axisSubdivision[2]; z++)
			{
				delete subdivisions[x][y][z];
			}
		}
	}

	this->_pointCloudScene = pointCloudScene;
	_pointCloudScene->_sceneGroup->addComponent(pointClouds[1]);
	calculateCloudDensity();

	readParameters("proceduralParameters.ini");
	createVoxelGrid();
	subdivideCloud();
	test();
}

ProceduralGenerator::~ProceduralGenerator()
{
	for (size_t x = 0; x < axisSubdivision[0]; x++)
	{
		for (size_t y = 0; y < axisSubdivision[1]; y++)
		{
			for (size_t z = 0; z < axisSubdivision[2]; z++)
			{
				delete subdivisions[x][y][z];
			}
		}
	}
}

/**
 * @brief Compute the cloud density
*/
void ProceduralGenerator::calculateCloudDensity()
{
	if (_pointCloudScene) {
		int numberPoints = _pointCloudScene->_pointCloud->getNumberOfPoints();
		glm::vec3 AABBSize = _pointCloudScene->getAABB().size();
		cloudDensity = numberPoints / (AABBSize.x * AABBSize.y * AABBSize.z);
		std::cout << "Cloud density: " << cloudDensity << std::endl;
	}
	else {
		std::cerr << "[ProceduralGenerator]->Nullptr in _pointCloudScene" << std::endl;
	}
}

/**
 * @brief Read the parameters from the parameters file
 * @param path where the parameters file is
*/
void ProceduralGenerator::readParameters(const std::string& path)
{
	std::ifstream parametersFile;
	parametersFile.open(path, std::ifstream::in);

	if (!parametersFile) {
		std::cerr << "[ProceduralGenerator]->Error while trying to open the parameters file. Path: " + path << std::endl;
	}
	else {
		std::string line;
		while (!parametersFile.eof()) {
			getline(parametersFile, line);
			if (line != "") {
				int pos = line.find("=");
				std::string parameter = line.substr(0, pos);
				std::string value = line.substr(++pos, line.length());
				if (parameter == "gsd")			   gsd = stof(value);
				else if (parameter == "expansion") expansion = stoi(value);
			}
		}
		parametersFile.close();
		glm::vec3 size = _pointCloudScene->_pointCloud->getAABB().size();
		for (size_t i = 0; i < 2; i++)
		{
			axisSubdivision[i] = round(size[i] / gsd);
			axisSubdivisionOriginal[i] = axisSubdivision[i];
		}
		//axisSubdivisionOriginal[0] = axisSubdivision[0] = 1000;
		//axisSubdivisionOriginal[1] = axisSubdivision[1] = 500;
		axisSubdivisionOriginal[2] = axisSubdivision[2] = 1;
	}
}

/**
 * @brief compute the height of a proceduralVoxel as the mean of the inmediate neightbour
 * @param x index of the subdivision
 * @param y index of the subdivision
*/
void ProceduralGenerator::meanHeight(unsigned x, unsigned y)
{
	float mean = 0;
	char counter = 0;
	int auxX, auxY;
	for (int i = -1; i < 2; i++)
	{
		for (int j = -1; j < 2; j++)
		{
			auxX = x + i;
			auxX = std::min((int)axisSubdivision[0] - 1, std::max(0, auxX));
			auxY = y + j;
			auxY = std::min((int)axisSubdivision[1] - 1, std::max(0, auxY));
			if (auxX != x || auxY != y) {
				counter++;
				float height = subdivisions[auxX][auxY][0]->getHeight();
				if (height != FLT_MAX)
					mean += height;
				else
					counter--;
			}
		}
	}
	if (counter > 0) {
		mean /= counter;
		subdivisions[x][y][0]->setHeight(mean);
	}
}


/**
 * @brief Initialize the voxel grid
*/
void ProceduralGenerator::createVoxelGrid()
{
	glm::vec3 size = _pointCloudScene->_pointCloud->getAABB().size();
	glm::vec3 minPoint = _pointCloudScene->_pointCloud->getAABB().min();
	float stride[3];

	for (unsigned i = 0; i < 3; i++)
	{
		stride[i] = size[i] / axisSubdivision[i];
		axisSubdivision[i] *= expansion;
		minPoint[i] *= expansion;
	}
	axisSubdivision[2] /= expansion;
	minPoint[2] /= expansion;

	subdivisions.resize(axisSubdivision[0]);
	for (size_t x = 0; x < axisSubdivision[0]; x++)
	{
		subdivisions[x].resize(axisSubdivision[1]);
		for (size_t y = 0; y < axisSubdivision[1]; y++)
		{
			subdivisions[x][y].resize(axisSubdivision[2]);
			for (size_t z = 0; z < axisSubdivision[2]; z++)
			{
				AABB* newAABB = new AABB;
				glm::vec3 point(minPoint);
				point[0] += stride[0] * x;
				point[1] += stride[1] * y;
				point[2] += stride[2] * z;
				newAABB->update(point);
				point[0] += stride[0];
				point[1] += stride[1];
				point[2] += stride[2];
				newAABB->update(point);
				ProceduralVoxel* procVoxel = new ProceduralVoxel(_pointCloudScene->_pointCloud, newAABB);
				subdivisions[x][y][z] = procVoxel;
				//_pointCloudScene->_sceneGroup->addComponent(procVoxel);
			}
		}
	}
}

/**
 * @brief Assign each point of the PointCloud to the corresponding voxel in the voxel grid
*/
void ProceduralGenerator::subdivideCloud()
{
	unsigned startPoint[3];
	for (size_t i = 0; i < 3; i++)
	{
		startPoint[i] = axisSubdivision[i] / 2 - axisSubdivisionOriginal[i] / 2;
	}

	unsigned pointCloudSize = this->_pointCloudScene->_pointCloud->getNumberOfPoints();
	std::vector<PointModel>* points = _pointCloudScene->_pointCloud->getPoints();

	glm::vec3 size = _pointCloudScene->_pointCloud->getAABB().size();
	glm::vec3 minPoint = _pointCloudScene->_pointCloud->getAABB().min();
	float stride[3];

	for (unsigned i = 0; i < 3; i++)
	{
		stride[i] = size[i] / axisSubdivisionOriginal[i];
	}

	unsigned x, y, z;
	glm::vec3 relativePoint;
	unsigned xOffset, yOffset, zOffset;
	xOffset = (axisSubdivision[0] - axisSubdivisionOriginal[0]) / 2;
	yOffset = (axisSubdivision[1] - axisSubdivisionOriginal[1]) / 2;
	zOffset = (axisSubdivision[2] - axisSubdivisionOriginal[2]) / 2;
	for (int i = 0; i < points->size(); i++)
	{
		relativePoint = (*points)[i]._point - minPoint;
		x = floor(relativePoint[0] / stride[0]) + xOffset;
		y = floor(relativePoint[1] / stride[1]) + yOffset;
		z = floor(relativePoint[2] / stride[2]) + zOffset;
		if (x == axisSubdivisionOriginal[0] + xOffset)
			x--;
		if (y == axisSubdivisionOriginal[1] + yOffset)
			y--;
		if (z == axisSubdivisionOriginal[2] + zOffset)
			z--;
		subdivisions[x][y][z]->addPoint(i);
	}

#pragma omp parallel for collapse(3)
	for (int x = startPoint[0]; x < startPoint[0] + axisSubdivisionOriginal[0]; x++)
	{
		for (int y = startPoint[1]; y < startPoint[1] + axisSubdivisionOriginal[1]; y++)
		{
			for (int z = startPoint[2]; z < startPoint[2] + axisSubdivisionOriginal[2]; z++)
			{
				subdivisions[x][y][z]->setProcedural(false);
				subdivisions[x][y][z]->computeHeight();
				subdivisions[x][y][z]->computeColor();
				//subdivisions[x][y][z]->checkPoints();
			}
		}
	}

	saveHeightMap();
	saveTextureMap();
}

/**
 * @brief Saves the current voxel grid as a png file in gray scale that represents a height map
*/
void ProceduralGenerator::saveHeightMap()
{
	float minPointZ = _pointCloudScene->_pointCloud->getAABB().min()[2];
	float relativeMaxPointZ = _pointCloudScene->_pointCloud->getAABB().max()[2] - minPointZ;
	float relativeHeightValue;
	float height;
	std::vector<unsigned char>* pixels = new std::vector<unsigned char>();
	for (int y = 0; y < axisSubdivision[1]; y++)
	{
		for (int x = 0; x < axisSubdivision[0]; x++)
		{
			unsigned char color;
			height = subdivisions[x][y][0]->getHeight();
			if (height != FLT_MAX)
				relativeHeightValue = (height - minPointZ) / relativeMaxPointZ;
			else
				relativeHeightValue = 0;
			color = std::min(255, int(relativeHeightValue * 256.0f));
			glm::vec3 aux = subdivisions[x][y][0]->getColor();
			pixels->push_back(color);
			pixels->push_back(color);
			pixels->push_back(color);
			pixels->push_back(255);

		}
	}
	Image* image = new Image(pixels->data(), axisSubdivision[0], axisSubdivision[1], 4);
	image->saveImage("heightmap.png");
}

/**
 * @brief Saves the current voxel grid as a png file in RGB scale that could be used as a texture of the terrain
*/
void ProceduralGenerator::saveTextureMap()
{
	float minPointZ = _pointCloudScene->_pointCloud->getAABB().min()[2];
	float relativeMaxPointZ = _pointCloudScene->_pointCloud->getAABB().max()[2] - minPointZ;
	float relativeHeightValue;
	glm::vec3 color;
	std::vector<unsigned char>* pixels = new std::vector<unsigned char>();
	for (int y = 0; y < axisSubdivision[1]; y++)
	{
		for (int x = 0; x < axisSubdivision[0]; x++)
		{
			color = subdivisions[x][y][0]->getColor();
			pixels->push_back(color[0]);
			pixels->push_back(color[1]);
			pixels->push_back(color[2]);
			pixels->push_back(255);

		}
	}
	Image* image = new Image(pixels->data(), axisSubdivision[0], axisSubdivision[1], 4);
	image->saveImage("texturemap.png");
}

void ProceduralGenerator::test()
{
	tinynurbs::Surface<float> srf;
	srf.degree_u = 5;
	srf.degree_v = 5;
	srf.knots_u.resize(axisSubdivision[0] + srf.degree_u + 1);
	srf.knots_v.resize(axisSubdivision[1] + srf.degree_v + 1);
	std::iota(srf.knots_u.begin(), srf.knots_u.end(), 0);
	std::iota(srf.knots_v.begin(), srf.knots_v.end(), 0);

	std::vector<glm::vec3> vec;
	for (int x = 0; x < axisSubdivision[0]; x++)
	{
		for (int y = 0; y < axisSubdivision[1]; y++)
		{
			glm::vec3 aux = subdivisions[x][y][0]->getMidPoint();
			if (aux[2] == FLT_MAX)
				meanHeight(x, y);
			aux = subdivisions[x][y][0]->getMidPoint();
			vec.push_back(aux);
		}
	}

	srf.control_points = { axisSubdivision[0], axisSubdivision[1], vec };
	if (tinynurbs::surfaceIsValid(srf)) {
		glm::vec3 minPoint = pointClouds[0]->getAABB().min();
		glm::vec3 maxPoint = pointClouds[0]->getAABB().max();
		PointModel point;
#pragma omp parallel for private(point)
		for (int x = 0; x < axisSubdivision[0] * 5; x++)
		{
			for (int y = 0; y < axisSubdivision[1] * 5; y++)
			{
				point._point = tinynurbs::surfacePoint(srf, x * 0.2f, y * 0.2f);
				point.saveRGB(subdivisions[floor(x / 5)][floor(y / 5)][0]->getColor());

#pragma omp critical
				pointClouds[1]->_points.push_back(point);

			}
		}

		this->pointClouds[1]->setVAOData();
	}
}
