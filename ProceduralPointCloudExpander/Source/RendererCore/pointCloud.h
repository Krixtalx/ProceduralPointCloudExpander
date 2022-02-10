#pragma once
#include "Model.h"
#include "GeometryUtils/AABB.h"


class PointCloud : public PPCX::Model {
private:
	std::vector<PointModel> vbo;
	AABB aabb;

	void newVBO(GLenum freqAct);

	void newIBO(const std::vector<GLuint>& data, GLenum freqAct);

public:
	bool needUpdating = false;

	PointCloud(std::string shaderProgram, const vec3& pos = { 0, 0, 0 });

	PointCloud(std::string shaderProgram, const std::vector<PointModel>& points, const AABB& aabb, const vec3& pos = { 0, 0, 0 });

	PointCloud(PointCloud& orig);

	~PointCloud();

	void newPoint(const PointModel& point);

	void newPoints(const std::vector<PointModel>& points);

	void updateCloud();

	void drawModel(const glm::mat4& MVPMatrix) override;

	std::vector<PointModel>& getPoints();

	unsigned getNumberOfPoints() const;

	const AABB& getAABB();

	float getDensity() const;
};

