#pragma once
#include <GeometryUtils/AABB.h>
class ProceduralVoxel{//:public Model3D{
private:
	AABB* aabb;
	DrawAABB* drawAABB;
	PointCloud* pointCloud;
	std::vector<unsigned> pointsIndex;
	bool procedural = true;
	static const bool wireframe = true;
	float height;
	glm::vec3 color;

public:

	ProceduralVoxel(PointCloud* pointCloud, AABB* aabb);

	~ProceduralVoxel();

	void addPoint(unsigned pointIndex);

	void drawAsLines(RenderingShader* shader, const RendEnum::RendShaderTypes shaderType, std::vector<mat4>& matrix);

	void setAABB(AABB* aabb);

	void setProcedural(bool proc);

	void computeHeight();

	void computeColor();
	
	void checkPoints();

	bool isInside(PointCloud::PointModel point);

	bool load(const mat4& modelMatrix);

	void setHeight(float h);

	float getHeight();

	glm::vec3 getMidPoint();

	glm::vec3 getColor();
};

