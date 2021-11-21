#include "stdafx.h"
#include "ProceduralVoxel.h"

ProceduralVoxel::ProceduralVoxel(PPCX::PointCloud* pointCloud, AABB* aabb):nube(pointCloud), aabb(aabb)
{
}

ProceduralVoxel::~ProceduralVoxel()
{
	delete aabb;
}

void ProceduralVoxel::addPoint(unsigned pointIndex)
{
	pointsIndex.push_back(pointIndex);
}

void ProceduralVoxel::setAABB(AABB* aabb)
{
	if (this->aabb)
		delete this->aabb;
	this->aabb = aabb;
}

void ProceduralVoxel::setProcedural(bool proc)
{
	this->procedural = proc;
}

void ProceduralVoxel::computeHeight()
{
	std::vector<PointModel> points = nube->getPoints();
	unsigned size = pointsIndex.size();
	height = 0;
	if (size != 0) {
		for (size_t i = 0; i < size; i++)
		{
			height += points[pointsIndex[i]]._point[2];
		}

		height /= size;
	}
	else {
		height = FLT_MAX;
	}
}

void ProceduralVoxel::computeColor()
{
	std::vector<PointModel> points = nube->getPoints();
	unsigned size = pointsIndex.size();
	color = { 0, 0, 0 };
	if (size != 0) {
		for (size_t i = 0; i < size; i++)
		{
			color += glm::vec3(points[pointsIndex[i]].getRGBVec3());
		}

		color /= size;
	}
	else {
		color = { 0, 0, 0 };
	}
}

void ProceduralVoxel::checkPoints()
{
	std::vector<PointModel> points = nube->getPoints();
	unsigned size = pointsIndex.size();
	if (size != 0) {
		for (size_t i = 0; i < size; i++)
		{
			if (!isInside(points[pointsIndex[i]])) {
				std::cout << "Outside point" << std::endl;
			}
		}
	}
}

bool ProceduralVoxel::isInside(PointModel point)
{
	return aabb->isInside(point._point);
}

void ProceduralVoxel::setHeight(float h)
{
	height = h;
}

float ProceduralVoxel::getHeight()
{
	return height;
}

glm::vec3 ProceduralVoxel::getMidPoint()
{
	glm::vec3 aux = aabb->center();
	aux[2] = height;
	return aux;
}

glm::vec3 ProceduralVoxel::getColor()
{
	return color;
}
