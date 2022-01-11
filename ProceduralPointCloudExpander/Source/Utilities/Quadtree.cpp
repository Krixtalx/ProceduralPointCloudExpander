#include "stdafx.h"
#include "Quadtree.h"

Quadtree::Quadtree(AABB aabb)
{
	root = std::make_unique<ProceduralVoxel>(aabb);
}

Quadtree::Quadtree(const Quadtree& orig)
{
	throw std::runtime_error("[Quadtree::Quadtree(copy)]: Not implemented");
}

Quadtree::~Quadtree() = default;

void Quadtree::insert(const unsigned& pointIndex) const
{
	root->addPoint(pointIndex);
}

void Quadtree::reset() {
	root.reset();
}

unsigned Quadtree::getDepth() {
	return root->getDepth(0);
}

