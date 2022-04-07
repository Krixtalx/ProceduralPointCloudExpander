//
// Created by Niskp on 08/10/2021.
//
#include "stdafx.h"
#include "Model.h"


PPCX::Model::Model(std::string shaderProgram, const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale) :
	idVAO(UINT_MAX), idVBO(UINT_MAX), idIBO(UINT_MAX),
	shaderProgram(std::move(shaderProgram)) {
	mModelado = glm::mat4(1.0f);
	mModelado = translate(mModelado, pos);
	mModelado = rotate(mModelado, glm::radians(rot.x), { 1, 0, 0 });
	mModelado = rotate(mModelado, glm::radians(rot.y), { 0, 1, 0 });
	mModelado = rotate(mModelado, glm::radians(rot.z), { 0, 0, 1 });
	mModelado = glm::scale(mModelado, scale);
	//Creamos nuestro VAO
	glGenVertexArrays(1, &idVAO);
	glBindVertexArray(idVAO);
}

/**
 * Constructor copia. Copia el numVertices y el shaderProgram y realiza una nueva instanciacion de los vbos e ibos
 * @param orig
 */
PPCX::Model::Model(Model& orig) : shaderProgram(orig.shaderProgram), mModelado(orig.mModelado) {
	//Creamos nuestro VAO
	glGenVertexArrays(1, &idVAO);
	glBindVertexArray(idVAO);

}

/**
 * Destructor. Libera todos los recursos reservados a OpenGL.
 */
PPCX::Model::~Model() {
	if (idVBO != UINT_MAX)
		glDeleteBuffers(1, &idVBO);
	if (idIBO != UINT_MAX)
		glDeleteBuffers(1, &idIBO);
	glDeleteVertexArrays(1, &idVAO);
}

bool& PPCX::Model::getVisibility() {
	return visible;
}
