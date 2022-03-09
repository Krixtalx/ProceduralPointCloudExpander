#include "stdafx.h"
#include "InstancedPointCloud.h"
#include <RendererCore/ShaderManager.h>

#include <utility>

InstancedPointCloud::InstancedPointCloud(std::string shaderProgram, const vec3& pos) : PointCloud(
	std::move(shaderProgram), pos),
	newInstanceUpdate(false) {}

InstancedPointCloud::InstancedPointCloud(std::string shaderProgram, const std::vector<PointModel>& points, const AABB& aabb, const vec3& pos) : PointCloud(
	std::move(shaderProgram), points, pos),
	newInstanceUpdate(false) {}

void InstancedPointCloud::newInstance(const glm::vec3& pos) {
	offsets.push_back(pos);
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
			PPCX::ShaderManager::getInstancia()->setUniform(this->shaderProgram, "matrizMVP", MVPMatrix);

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
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * offsets.size(), offsets.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3),
						  nullptr);
	glVertexAttribDivisor(2, 1);
	newInstanceUpdate = false;
}