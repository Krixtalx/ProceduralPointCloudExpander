#include "stdafx.h"
#include "Quadtree.h"

Quadtree::Quadtree(AABB aabb) {
	root = std::make_unique<ProceduralVoxel>(aabb);
}

Quadtree::Quadtree(const Quadtree& orig) {
	throw std::runtime_error("[Quadtree::Quadtree(copy)]: Not implemented");
}

Quadtree::~Quadtree() = default;

void Quadtree::insert(const unsigned& pointIndex) const {
	root->addPoint(pointIndex);
}

void Quadtree::reset() {
	root.reset();
}

unsigned Quadtree::getDepth() {
	return root->getDepth(0);
}

unsigned Quadtree::getNumSubdivisions() {
	return root->numSubdivisions;
}

std::vector<std::shared_ptr<ProceduralVoxel>> Quadtree::getLeafs() const {
	std::vector<std::shared_ptr<ProceduralVoxel>> leafs;
	root->getAllLeafs(leafs);
	return leafs;
}

