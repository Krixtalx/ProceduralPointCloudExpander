//
// Created by Niskp on 08/10/2021.
//
#include "stdafx.h"
#include "pointCloud.h"
#include "ShaderManager.h"

/**
 * Constructor parametrizado
 * @param shaderProgram que se usará para renderizar el modelo
 * @param numVertices que contiene el modelo
 */
PAG::pointCloud::pointCloud(std::string shaderProgram, glm::vec3 pos) : shaderProgram(
	std::move(shaderProgram)),
	posicion(pos) {
	//Creamos nuestro VAO
	glGenVertexArrays(1, &idVAO);
	glBindVertexArray(idVAO);

	//Ponemos tanto espacios en el vector de VBO como parametros tengamos para el shader.
	idVBO = UINT_MAX;
	idIBO = UINT_MAX;
}

/**
 * Constructor copia. Copia el numVertices y el shaderProgram y realiza una nueva instanciacion de los vbos e ibos
 * @param orig
 */
PAG::pointCloud::pointCloud(PAG::pointCloud& orig) : shaderProgram(orig.shaderProgram),
vbo(orig.vbo), ibo(orig.ibo) {
	//Creamos nuestro VAO
	glGenVertexArrays(1, &idVAO);
	glBindVertexArray(idVAO);


	nuevoVBO(vbo, GL_STATIC_DRAW);

	nuevoIBO(ibo, GL_STATIC_DRAW);
}

/**
 * Destructor. Libera todos los recursos reservados a OpenGL.
 */
PAG::pointCloud::~pointCloud() {
	if (idVBO != UINT_MAX)
		glDeleteBuffers(1, &idVBO);
	if (idIBO != UINT_MAX)
		glDeleteBuffers(1, &idIBO);

	glDeleteVertexArrays(1, &idVAO);
}

/**
 * Instancia un VBO en el contexto OpenGL y lo guarda en vbos
 * @param tipoDato parametro del shader que representa el vbo
 * @param datos a instanciar
 * @param freqAct GLenum que indica con que frecuencia se van a modificar los vertices. GL_STATIC_DRAW siempre por ahora
 */
void PAG::pointCloud::nuevoVBO(std::vector<PointModel> datos, GLenum freqAct) {
	//Si hay un buffer de este tipo instanciado, lo eliminamos
	if (idVBO != UINT_MAX) {
		glDeleteBuffers(1, &idVBO);
	}
	vbo = datos;
	glBindVertexArray(idVAO);
	glGenBuffers(1, &idVBO);
	glBindBuffer(GL_ARRAY_BUFFER, idVBO);
	glBufferData(GL_ARRAY_BUFFER, datos.size() * sizeof(PointModel), datos.data(), freqAct);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PointModel),
		nullptr);
	
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(PointModel), ((GLubyte*)nullptr + sizeof(glm::vec3)));
}

/**
 * Instancia un IBO en el contexto OpenGL y lo guarda en ibos
 * @param modo modo de dibujado que representa el ibo
 * @param datos a instanciar
 * @param freqAct GLenum que indica con que frecuencia se van a modificar los vertices. GL_STATIC_DRAW siempre por ahora
 */
void PAG::pointCloud::nuevoIBO(std::vector<GLuint> datos, GLenum freqAct) {
	//Si hay un buffer de este tipo instanciado, lo eliminamos
	if (idIBO != UINT_MAX) {
		glDeleteBuffers(1, &idIBO);
	}
	ibo = datos;
	glBindVertexArray(idVAO);
	glGenBuffers(1, &idIBO);
	glBindBuffer(GL_ARRAY_BUFFER, idIBO);
	glBufferData(GL_ARRAY_BUFFER, datos.size() * sizeof(GLuint), datos.data(), freqAct);
}

/**
 * Función a la que se llama cuando se debe de dibujar el modelo
 * @param modo modo de dibujado a usar
 */
void PAG::pointCloud::dibujarModelo(glm::mat4 matrizMVP) {
	try {
		matrizMVP = matrizMVP * glm::translate(posicion);
		PAG::ShaderManager::getInstancia()->activarSP(shaderProgram);
		PAG::ShaderManager::getInstancia()->setUniform(this->shaderProgram, "matrizMVP", matrizMVP);

		glBindVertexArray(idVAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idIBO);
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		glDrawElements(GL_TRIANGLES, ibo.size(), GL_UNSIGNED_INT, nullptr);
	}
	catch (std::runtime_error& e) {
		throw e;
	}
}
