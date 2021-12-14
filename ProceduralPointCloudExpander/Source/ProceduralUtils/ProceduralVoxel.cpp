#include "stdafx.h"
#include "ProceduralVoxel.h"

ProceduralVoxel::ProceduralVoxel(PPCX::PointCloud* pointCloud, AABB* aabb) :aabb(aabb), nube(pointCloud) {}

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
			color += vec3(points[pointsIndex[i]].getRGBVec3());
		}
		color /= size;
	}
}

void ProceduralVoxel::checkPoints() const {
	const std::vector<PointModel> points = nube->getPoints();
	if (const unsigned size = pointsIndex.size(); size != 0) {
		for (size_t i = 0; i < size; i++) {
			if (!isInside(points[pointsIndex[i]])) {
				std::cout << "Outside point" << std::endl;
			}
		}
	}
}

bool ProceduralVoxel::isInside(PointModel point) const {
	return aabb->isInside(point._point);
}

void ProceduralVoxel::setHeight(float h) {
	height = h;
}

void ProceduralVoxel::setColor(vec3 color) {
	this->color = color;
}

float ProceduralVoxel::getHeight() const {
	return height;
}

vec3 ProceduralVoxel::getRepresentativePoint() const {
	vec3 aux = aabb->min();
	aux[2] = height;
	return aux;
}

vec3 ProceduralVoxel::getColor() const {
	return color;
}

unsigned ProceduralVoxel::getNumberOfPoints() const {
	return pointsIndex.size();
}

unsigned ProceduralVoxel::numberPointsToDensity(float density) const {
	vec3 size = aabb->size();
	unsigned number = size.x * size.y * density;
	if (number > pointsIndex.size())
		return number - pointsIndex.size();
	return 0;
}
