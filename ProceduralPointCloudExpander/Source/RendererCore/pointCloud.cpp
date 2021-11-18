//
// Created by Niskp on 08/10/2021.
//
#include "stdafx.h"
#include "PointCloud.h"
#include "ShaderManager.h"

/**
 * Constructor parametrizado
 * @param shaderProgram que se usará para renderizar el modelo
 * @param pos Posicion inicial de la nube de puntos
 */
PPCX::PointCloud::PointCloud(std::string shaderProgram, glm::vec3 pos) : idVBO(UINT_MAX), idIBO(UINT_MAX),
                                                                         shaderProgram(
	                                                                         std::move(shaderProgram)),
                                                                         posicion(pos)
{
	//Creamos nuestro VAO
	glGenVertexArrays(1, &idVAO);
	glBindVertexArray(idVAO);

	//Ponemos tanto espacios en el vector de VBO como parametros tengamos para el shader.
}

/**
 * Constructor copia. Copia el numVertices y el shaderProgram y realiza una nueva instanciacion de los vbos e ibos
 * @param orig
 */
PPCX::PointCloud::PointCloud(PointCloud& orig) : vbo(orig.vbo),
                                                 ibo(orig.ibo), shaderProgram(orig.shaderProgram)
{
	//Creamos nuestro VAO
	glGenVertexArrays(1, &idVAO);
	glBindVertexArray(idVAO);


	nuevoVBO(vbo, GL_STATIC_DRAW);

	nuevoIBO(ibo, GL_STATIC_DRAW);
}

/**
 * Destructor. Libera todos los recursos reservados a OpenGL.
 */
PPCX::PointCloud::~PointCloud()
{
	if (idVBO != UINT_MAX)
		glDeleteBuffers(1, &idVBO);
	if (idIBO != UINT_MAX)
		glDeleteBuffers(1, &idIBO);

	glDeleteVertexArrays(1, &idVAO);
}

void PPCX::PointCloud::nuevoPunto(const PointModel& punto)
{
	vbo.push_back(punto);
}

void PPCX::PointCloud::nuevosPuntos(const std::vector<PointModel>& puntos)
{
	vbo.resize(vbo.size() + puntos.size());
	std::copy(puntos.begin(), puntos.end(), vbo.begin() + vbo.size());
}

/**
 * Instancia un VBO en el contexto OpenGL y lo guarda en vbos
 * @param datos a instanciar
 * @param freqAct GLenum que indica con que frecuencia se van a modificar los vertices. GL_STATIC_DRAW siempre por ahora
 */
void PPCX::PointCloud::nuevoVBO(std::vector<PointModel> datos, GLenum freqAct)
{
	//Si hay un buffer de este tipo instanciado, lo eliminamos
	if (idVBO != UINT_MAX)
	{
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
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(PointModel),
	                      (static_cast<GLubyte*>(nullptr) + sizeof(glm::vec3)));
}

/**
 * Instancia un IBO en el contexto OpenGL y lo guarda en ibos
 * @param datos a instanciar
 * @param freqAct GLenum que indica con que frecuencia se van a modificar los vertices. GL_STATIC_DRAW siempre por ahora
 */
void PPCX::PointCloud::nuevoIBO(std::vector<GLuint> datos, GLenum freqAct)
{
	//Si hay un buffer de este tipo instanciado, lo eliminamos
	if (idIBO != UINT_MAX)
	{
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
 */
void PPCX::PointCloud::dibujarModelo(glm::mat4 matrizMVP)
{
	try
	{
		matrizMVP = matrizMVP * translate(posicion);
		ShaderManager::getInstancia()->activarSP(shaderProgram);
		ShaderManager::getInstancia()->setUniform(this->shaderProgram, "matrizMVP", matrizMVP);

		glBindVertexArray(idVAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idIBO);
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		glDrawElements(GL_TRIANGLES, ibo.size(), GL_UNSIGNED_INT, nullptr);
	}
	catch (std::runtime_error& e)
	{
		throw;
	}
}
