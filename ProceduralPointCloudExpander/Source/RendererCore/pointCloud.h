#pragma once
#include "Model.h"
#include "GeometryUtils/AABB.h"
#include "Utilities/Point.h"


class PointCloud : public PPCX::Model {
protected:
	std::vector<PointModel> vbo;
	AABB aabb;

	void newVBO(GLenum freqAct);

	void newIBO(const std::vector<GLuint>& data, GLenum freqAct);

public:
	bool needUpdating = false;

	PointCloud(std::string shaderProgram, const glm::vec3& pos = { 0, 0, 0 }, const glm::vec3& rot = { 0, 0, 0 }, const glm::vec3& scale = { 1, 1, 1 });

	PointCloud(std::string shaderProgram, const std::vector<PointModel>& points, const AABB& aabb, const glm::vec3& pos = { 0, 0, 0 }, const glm::vec3& rot = { 0, 0, 0 }, const glm::vec3& scale = { 1, 1, 1 });

	PointCloud(PointCloud& orig);

	~PointCloud() override;

	void newPoint(const PointModel& point);

	void newPoints(const std::vector<PointModel>& points);

	void updateCloud();

	virtual void drawModel(const glm::mat4& MVPMatrix) override;

	std::vector<PointModel>& getPoints();

	unsigned getNumberOfPoints() const;

	const AABB& getAABB();

	float getDensity() const;

	glm::vec3 getRandomPointColor();
};

