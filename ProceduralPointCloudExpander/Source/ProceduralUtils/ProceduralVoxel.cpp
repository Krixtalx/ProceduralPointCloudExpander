#include "stdafx.h"
#include "ProceduralVoxel.h"

PointCloud* ProceduralVoxel::cloud = nullptr;
unsigned ProceduralVoxel::numSubdivisions = 0;

ProceduralVoxel::ProceduralVoxel(const AABB& aabb) :aabb(aabb) {}

ProceduralVoxel::~ProceduralVoxel() = default;

void ProceduralVoxel::addPoint(const unsigned pointIndex) {
	if (leaf && pointsIndex.size() < 500) {
		pointsIndex.push_back(pointIndex);
	} else {
		const glm::vec3 midPoint = aabb.center();
		const std::vector<PointModel>& points = cloud->getPoints();
		unsigned node = 0;
		if (leaf) {
			numSubdivisions++;
			std::vector<AABB> subAABBs = aabb.split(2, 2);
			for (int i = 0; i < 4; ++i) {
				children[i] = std::make_shared<ProceduralVoxel>(subAABBs[i]);
			}

			for (int i = 0; i < pointsIndex.size(); ++i) {
				node = (points[i]._point.x >= midPoint.x) << 1;
				node |= (points[i]._point.y >= midPoint.y);
				children[node]->addPoint(i);
			}
			pointsIndex.clear();
			pointsIndex.resize(0);
			leaf = false;
		}
		node = (points[pointIndex]._point.x >= midPoint.x) << 1;
		node |= (points[pointIndex]._point.y >= midPoint.y);
		children[node]->addPoint(pointIndex);
	}
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
	return aabb.isInside(point._point);
}

void ProceduralVoxel::setHeight(float h) {
	height = h;
}

void ProceduralVoxel::setColor(vec3 color) {
	this->color = color;
}

void ProceduralVoxel::setPointCloud(PointCloud * cloud) {
	ProceduralVoxel::cloud = cloud;
}

float ProceduralVoxel::getHeight() const {
	return height;
}

vec3 ProceduralVoxel::getRepresentativePoint() const {
	vec3 aux = aabb.min();
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
	const vec3 size = aabb.size();
	const unsigned number = size.x * size.y * density;
	if (number > pointsIndex.size())
		return number - pointsIndex.size();
	return 0;
}

unsigned ProceduralVoxel::getDepth(unsigned currentDepth) {
	if (leaf)
		return currentDepth + 1;
	else {
		for (size_t i = 0; i < 4; i++) {
			unsigned value = children[i]->getDepth(currentDepth + 1);
			if (value > currentDepth)
				currentDepth = value;
		}
		return currentDepth;
	}
}

void ProceduralVoxel::getAllLeafs(std::vector<std::shared_ptr<ProceduralVoxel>>&leafs) const {
	for (const auto& child : children) {
		if (child->leaf) {
			leafs.push_back(child);
		} else {
			child->getAllLeafs(leafs);
		}
	}
}
