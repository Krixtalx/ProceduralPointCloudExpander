#include "stdafx.h"
#include "InstancedPointCloud.h"
#include <RendererCore/ShaderManager.h>

#include <utility>

InstancedPointCloud::InstancedPointCloud(std::string shaderProgram, const vec3& pos, const glm::vec3& rot, const glm::vec3& scale) : PointCloud(
	std::move(shaderProgram), pos, rot, scale),
	newInstanceUpdate(false) {}

InstancedPointCloud::InstancedPointCloud(std::string shaderProgram, const std::vector<PointModel>& points, const AABB& aabb, const vec3& pos, const glm::vec3& rot, const glm::vec3& scale) : PointCloud(std::move(shaderProgram), points, aabb, pos, rot, scale),
newInstanceUpdate(false) {}

void InstancedPointCloud::newInstance(const glm::vec3& position, const glm::vec3& rot, const glm::vec3& scale) {
	glm::mat4 matrix = glm::mat4(1.0f);
	matrix = glm::translate(matrix, position);
	matrix = glm::rotate(matrix, glm::radians(rot.x), { 1, 0, 0 });
	matrix = glm::rotate(matrix, glm::radians(rot.y), { 0, 1, 0 });
	matrix = glm::rotate(matrix, glm::radians(rot.z), { 0, 0, 1 });
	matrix = glm::scale(matrix, scale);

	offsets.push_back(matrix);
	newInstanceUpdate = true;
}

void InstancedPointCloud::drawModel(const glm::mat4& MVPMatrix) {
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

unsigned InstancedPointCloud::getNumberOfInstances() const {
	return offsets.size();
}

void InstancedPointCloud::updateInstancingData() {
	if (instancingVBO != UINT_MAX) {
		glDeleteBuffers(1, &instancingVBO);
	}
	glBindVertexArray(idVAO);
	glGenBuffers(1, &instancingVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instancingVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * offsets.size(), offsets.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), nullptr);
	glVertexAttribDivisor(2, 1);

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), reinterpret_cast<void*>(sizeof(glm::vec4)));
	glVertexAttribDivisor(3, 1);

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), reinterpret_cast<void*>(2 * sizeof(glm::vec4)));
	glVertexAttribDivisor(4, 1);

	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), reinterpret_cast<void*>(3 * sizeof(glm::vec4)));
	glVertexAttribDivisor(5, 1);
	newInstanceUpdate = false;
}