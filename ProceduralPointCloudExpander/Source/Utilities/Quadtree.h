#pragma once
#include "ProceduralUtils/ProceduralVoxel.h"

/**
* Specific implementation of a Quadtree for Procedural Point Cloud Expander.
*/
class Quadtree{

	std::unique_ptr<ProceduralVoxel> root;

public:
	Quadtree(AABB aabb);
	Quadtree(const Quadtree& orig);
	~Quadtree();

	void insert(const unsigned& pointIndex) const;
	void reset();
	unsigned getDepth();
	unsigned getNumSubdivisions();
	std::vector<std::shared_ptr<ProceduralVoxel>> getLeafs() const;

};