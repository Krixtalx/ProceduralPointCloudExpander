#pragma once
#include "Model.h"
#include "GeometryUtils/AABB.h"
#include "Utilities/Point.h"


class PointCloud : public PPCX::Model {
protected:
	std::vector<PointModel> vbo;
	AABB aabb;
	bool optimized;

	void newVBO(GLenum freqAct);

	void newIBO(const std::vector<GLuint>& data, GLenum freqAct);

public:
	bool needUpdating = false;

	PointCloud(std::string shaderProgram, const vec3& pos = { 0, 0, 0 }, const vec3& rot = { 0, 0, 0 }, const vec3& scale = { 1, 1, 1 });

	PointCloud(std::string shaderProgram, const std::vector<PointModel>& points, const AABB& aabb, const vec3& pos = { 0, 0, 0 }, const
	           vec3& rot = { 0, 0, 0 }, const vec3& scale = { 1, 1, 1 });

	PointCloud(PointCloud& orig);

	~PointCloud() override;

	void newPoint(const PointModel& point);

	void newPoints(const std::vector<PointModel>& points);

	void updateCloud();

	virtual void drawModel(const mat4& MVPMatrix) override;

	virtual std::vector<PointModel>& getPoints();

	unsigned getNumberOfPoints() const;

	const AABB& getAABB();

	float getDensity() const;

	vec3 getRandomPointColor();

	void optimize();
};

