#include "stdafx.h"
#include "InstancedPointCloud.h"
#include <RendererCore/ShaderManager.h>

#include <utility>

InstancedPointCloud::InstancedPointCloud(std::string shaderProgram, const vec3& pos, const vec3& rot, const vec3& scale) : PointCloud(
	std::move(shaderProgram), pos, rot, scale),
	newInstanceUpdate(false) {}

InstancedPointCloud::InstancedPointCloud(std::string shaderProgram, const std::vector<PointModel>& points, const AABB& aabb, const vec3& pos, const
	vec3& rot, const vec3& scale) : PointCloud(std::move(shaderProgram), points, aabb, pos, rot, scale),
	newInstanceUpdate(false) {}

void InstancedPointCloud::newInstance(const vec3& position, const vec3& rot, const vec3& scale) {
	mat4 matrix = mat4(1.0f);
	matrix = translate(matrix, position);
	matrix = rotate(matrix, radians(rot.x), { 1, 0, 0 });
	matrix = rotate(matrix, radians(rot.y), { 0, 1, 0 });
	matrix = rotate(matrix, radians(rot.z), { 0, 0, 1 });
	matrix = glm::scale(matrix, scale);

	offsets.push_back(matrix);
	newInstanceUpdate = true;
	HQRNeedUpdate = true;
}

void InstancedPointCloud::drawModel(const mat4& MVPMatrix) {
	if (visible) {
		if (needUpdating)
			updateCloud();
		if (newInstanceUpdate)
			updateInstancingData();
		try {
			PPCX::ShaderManager::getInstancia()->activarSP(shaderProgram);
			PPCX::ShaderManager::getInstancia()->setUniform(this->shaderProgram, "matrizVP", MVPMatrix);

			glBindVertexArray(idVAO);
			glBindBuffer(GL_ARRAY_BUFFER, instancingVBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idIBO);
			glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
			glDrawElementsInstanced(GL_POINTS, vbo.size(), GL_UNSIGNED_INT, nullptr, offsets.size()); //Al ser una nube de puntos vbo.size = ibo.size
		} catch (std::runtime_error& e) {
			throw;
		}
	}
}

void InstancedPointCloud::resetInstances() {
	offsets.clear();
}

unsigned InstancedPointCloud::getNumberOfInstances() const {
	return offsets.size();
}

std::vector<PointModel>& InstancedPointCloud::getAllInstancesPoints() {
	allPoints.clear();
	allPoints.reserve(offsets.size() * vbo.size());
	for (const auto& offset : offsets) {
		for (const auto& point : vbo) {
			PointModel newPoint(point);
			//std::cout << newPoint._point.x << "-" << newPoint._point.y << "-" << newPoint._point.z << std::endl;
			newPoint._point = offset * vec4(point._point, 1);
			//std::cout << newPoint._point.x << "-" << newPoint._point.y << "-" << newPoint._point.z << std::endl << std::endl;
			allPoints.push_back(newPoint);

		}
	}
	return allPoints;
}

std::vector<mat4>& InstancedPointCloud::getOffsets() {
	return offsets;
}

void InstancedPointCloud::updateInstancingData() {
	if (instancingVBO != UINT_MAX) {
		glDeleteBuffers(1, &instancingVBO);
	}
	glBindVertexArray(idVAO);
	glGenBuffers(1, &instancingVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instancingVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(mat4) * offsets.size(), offsets.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(vec4), nullptr);
	glVertexAttribDivisor(2, 1);

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(vec4), reinterpret_cast<void*>(sizeof(vec4)));
	glVertexAttribDivisor(3, 1);

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(vec4), reinterpret_cast<void*>(2 * sizeof(vec4)));
	glVertexAttribDivisor(4, 1);

	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(vec4), reinterpret_cast<void*>(3 * sizeof(vec4)));
	glVertexAttribDivisor(5, 1);
	newInstanceUpdate = false;
}


unsigned InstancedPointCloud::getNumberOfPoints() const {
	return vbo.size() * offsets.size();
}

unsigned InstancedPointCloud::getOneInstanceNumberOfPoints() const {
	return vbo.size();
}
