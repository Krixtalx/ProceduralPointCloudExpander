#include "stdafx.h"
#include "ProceduralVoxel.h"

ProceduralVoxel::ProceduralVoxel(PointCloud* pointCloud, AABB* aabb) :aabb(aabb), cloud(pointCloud) {}

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
		auto& points = cloud->getPoints();
		for (size_t i = 0; i < size; i++) {
			height += points[pointsIndex[i]]._point[2];
		}
		height /= size;
	}
}

void ProceduralVoxel::computeColor() {
	const unsigned size = pointsIndex.size();
	if (size != 0) {
		auto& points = cloud->getPoints();
		for (size_t i = 0; i < size; i++) {
			color += vec3(points[pointsIndex[i]].getRGBVec3());
		}
		color /= size;
	}
}

void ProceduralVoxel::checkPoints() const {
	const std::vector<PointModel> points = cloud->getPoints();
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

void ProceduralVoxel::setVegetationMark() {
	vegetationMark = true;
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

vec3 ProceduralVoxel::getCenter() const {
	return aabb->center();
}

unsigned ProceduralVoxel::getNumberOfPoints() const {
	return pointsIndex.size();
}

float ProceduralVoxel::getDensity() const {
	const vec3 size = aabb->size();
	return pointsIndex.size() / (size.x * size.y);
}

bool ProceduralVoxel::getVegetationMark() const {
	return vegetationMark;
}

AABB ProceduralVoxel::getAABB() const {
	return *aabb;
}

unsigned ProceduralVoxel::numberPointsToDensity(const float density) const {
	const vec3 size = aabb->size();
	const unsigned number = size.x * size.y * density;
	if (number > pointsIndex.size())
		return number - pointsIndex.size();
	return 0;
}

std::vector<float> ProceduralVoxel::internalDistribution(const unsigned divX, const unsigned divY) const {
	std::vector<float> distribution;
	distribution.resize(divX * divY);
	const auto& points = cloud->getPoints();
	const float xSize = aabb->size().x / divX;
	const float ySize = aabb->size().y / divY;
	const vec3 minPoint = aabb->min();
	unsigned currentMax = 0;
	for (unsigned i : pointsIndex) {
		unsigned posX = (points[i]._point.x - minPoint.x) / xSize;
		posX = std::min(posX, divX - 1);
		unsigned posY = (points[i]._point.y - minPoint.y) / ySize;
		posY = std::min(posY, divY - 1);
		distribution[posX * divY + posY] += 1;
		if (distribution[posX * divY + posY] > currentMax)
			currentMax = distribution[posX * divY + posY];
	}
	unsigned i = 0;
	float aux = 0;
	for (float& val : distribution) {
		/*if (i++ % divY == 0)
			std::cout << std::endl;
		std::cout << val << " ";*/
		val = (currentMax - val + 1) / (pointsIndex.size());
		aux += val;
	}
	//std::cout << std::endl;
	i = 0;
	for (float& val : distribution) {
		val /= aux;
		/*if (i++ % divY == 0)
			std::cout << std::endl;
		std::cout << val << " ";*/
	}
	//std::cout << std::endl << std::endl;
	return distribution;
}
