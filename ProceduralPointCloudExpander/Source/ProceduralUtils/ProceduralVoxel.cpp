#include "stdafx.h"
#include "ProceduralVoxel.h"

ProceduralVoxel::ProceduralVoxel(PPCX::PointCloud* pointCloud, AABB* aabb) :nube(pointCloud), aabb(aabb) {}

ProceduralVoxel::~ProceduralVoxel() {
	delete aabb;
}

void ProceduralVoxel::addPoint(unsigned pointIndex) {
	pointsIndex.push_back(pointIndex);
}

void ProceduralVoxel::setAABB(AABB* aabb) {
	delete this->aabb;
	this->aabb = aabb;
}

void ProceduralVoxel::computeHeight() {
	const unsigned size = pointsIndex.size();
	if (size != 0) {
		height = 0;
		auto& points = nube->getPoints();
		for (size_t i = 0; i < size; i++) {
			height += points[pointsIndex[i]]._point[2];
		}
		height /= size;
	}
}

void ProceduralVoxel::computeColor() {
	const unsigned size = pointsIndex.size();
	if (size != 0) {
		auto& points = nube->getPoints();
		for (size_t i = 0; i < size; i++) {
			color += glm::vec3(points[pointsIndex[i]].getRGBVec3());
		}
		color /= size;
	}
}

void ProceduralVoxel::checkPoints() {
	const std::vector<PointModel> points = nube->getPoints();
	if (const unsigned size = pointsIndex.size(); size != 0) {
		for (size_t i = 0; i < size; i++) {
			if (!isInside(points[pointsIndex[i]])) {
				std::cout << "Outside point" << std::endl;
			}
		}
	}
}

bool ProceduralVoxel::isInside(PointModel point) {
	return aabb->isInside(point._point);
}

void ProceduralVoxel::setHeight(float h) {
	height = h;
}

void ProceduralVoxel::setColor(glm::vec3 color) {
	this->color = color;
}

float ProceduralVoxel::getHeight() {
	return height;
}

glm::vec3 ProceduralVoxel::getMidPoint() {
	glm::vec3 aux = aabb->center();
	aux[2] = height;
	return aux;
}

glm::vec3 ProceduralVoxel::getColor() {
	return color;
}

unsigned ProceduralVoxel::getNumberOfPoints() {
	return pointsIndex.size();
}

unsigned ProceduralVoxel::numberPointsToDensity(float density) {
	glm::vec3 size = aabb->size();
	unsigned number = size.x * size.y * density;
	if (number > pointsIndex.size())
		return number - pointsIndex.size();
	return 0;
}
