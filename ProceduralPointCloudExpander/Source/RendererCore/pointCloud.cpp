//
// Created by Niskp on 08/10/2021.
//
#include "stdafx.h"
#include "PointCloud.h"
#include "ShaderManager.h"


PPCX::PointCloud::PointCloud(std::string shaderProgram, const vec3& pos) :
	idVAO(UINT_MAX), idVBO(UINT_MAX), idIBO(UINT_MAX),
	shaderProgram(std::move(shaderProgram)),
	posicion(pos) {
	//Creamos nuestro VAO
	glGenVertexArrays(1, &idVAO);
	glBindVertexArray(idVAO);
}

/**
 * Constructor parametrizado
 * @param shaderProgram que se usará para renderizar el modelo
 * @param pos Posicion inicial de la nube de puntos
 */
PPCX::PointCloud::PointCloud(std::string shaderProgram, const std::vector<PointModel>& puntos, const AABB& aabb, const
                             vec3& pos) :
	idVAO(UINT_MAX), idVBO(UINT_MAX), idIBO(UINT_MAX),
	shaderProgram(std::move(shaderProgram)),
	posicion(pos),
	aabb(aabb), needUpdating(true)
{

	nuevosPuntos(puntos);
}

/**
 * Constructor copia. Copia el numVertices y el shaderProgram y realiza una nueva instanciacion de los vbos e ibos
 * @param orig
 */
PPCX::PointCloud::PointCloud(PointCloud& orig) : vbo(orig.vbo), shaderProgram(orig.shaderProgram) {
	//Creamos nuestro VAO
	glGenVertexArrays(1, &idVAO);
	glBindVertexArray(idVAO);

	nuevoVBO(GL_STATIC_DRAW);
}

/**
 * Destructor. Libera todos los recursos reservados a OpenGL.
 */
PPCX::PointCloud::~PointCloud() {
	if (idVBO != UINT_MAX)
		glDeleteBuffers(1, &idVBO);
	if (idIBO != UINT_MAX)
		glDeleteBuffers(1, &idIBO);

	glDeleteVertexArrays(1, &idVAO);
}

void PPCX::PointCloud::nuevoPunto(const PointModel& punto) {
	vbo.push_back(punto);
	aabb.update(punto._point);
}

void PPCX::PointCloud::nuevosPuntos(const std::vector<PointModel>& puntos) {
	vbo.clear();
	vbo.resize(puntos.size());
	std::copy(puntos.begin(), puntos.end(), vbo.begin());
}

void PPCX::PointCloud::actualizarNube() {
	if (idVAO == UINT_MAX) {
		glGenVertexArrays(1, &idVAO);
		glBindVertexArray(idVAO);
	}
	nuevoVBO(GL_STATIC_DRAW);
	std::vector<unsigned> ibo;
	ibo.resize(vbo.size());
	std::iota(ibo.begin(), ibo.end(), 0);
	nuevoIBO(ibo, GL_STATIC_DRAW);
	needUpdating = false;
}

/**
 * Instancia un VBO en el contexto OpenGL y lo guarda en vbos
 * @param datos a instanciar
 * @param freqAct GLenum que indica con que frecuencia se van a modificar los vertices. GL_STATIC_DRAW siempre por ahora
 */
void PPCX::PointCloud::nuevoVBO(GLenum freqAct) {
	//Si hay un buffer de este tipo instanciado, lo eliminamos
	if (idVBO != UINT_MAX) {
		glDeleteBuffers(1, &idVBO);
	}

	glBindVertexArray(idVAO);
	glGenBuffers(1, &idVBO);
	glBindBuffer(GL_ARRAY_BUFFER, idVBO);
	glBufferData(GL_ARRAY_BUFFER, vbo.size() * sizeof(PointModel), vbo.data(), freqAct);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PointModel),
						  nullptr);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(PointModel),
						  (static_cast<GLubyte*>(nullptr) + sizeof(vec3)));
}

/**
 * Instancia un IBO en el contexto OpenGL y lo guarda en ibos
 * @param datos a instanciar
 * @param freqAct GLenum que indica con que frecuencia se van a modificar los vertices. GL_STATIC_DRAW siempre por ahora
 */
void PPCX::PointCloud::nuevoIBO(const std::vector<GLuint>& datos, const GLenum freqAct) {
	//Si hay un buffer de este tipo instanciado, lo eliminamos
	if (idIBO != UINT_MAX) {
		glDeleteBuffers(1, &idIBO);
	}

	glBindVertexArray(idVAO);
	glGenBuffers(1, &idIBO);
	glBindBuffer(GL_ARRAY_BUFFER, idIBO);
	glBufferData(GL_ARRAY_BUFFER, datos.size() * sizeof(GLuint), datos.data(), freqAct);
}


/**
 * Función a la que se llama cuando se debe de dibujar el modelo
 */
void PPCX::PointCloud::dibujarModelo(const mat4 matrizMVP) const {
	if (visible) {
		try {
			//matrizMVP = matrizMVP * translate(posicion);
			ShaderManager::getInstancia()->activarSP(shaderProgram);
			ShaderManager::getInstancia()->setUniform(this->shaderProgram, "matrizMVP", matrizMVP);

			glBindVertexArray(idVAO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idIBO);
			glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
			glDrawElements(GL_POINTS, vbo.size(), GL_UNSIGNED_INT, nullptr); //Al ser una nube de puntos vbo.size = ibo.size
		} catch (std::runtime_error& e) {
			throw;
		}
	}
}

std::vector<PointModel>& PPCX::PointCloud::getPoints() {
	return vbo;
}

unsigned PPCX::PointCloud::getNumberOfPoints() const {
	return vbo.size();
}

const AABB& PPCX::PointCloud::getAABB() {
	return aabb;
}

float PPCX::PointCloud::getDensity() const {
	const int numberPoints = vbo.size();
	const vec3 AABBSize = aabb.size();
	std::cout<<AABBSize.x<<std::endl;
	std::cout<<AABBSize.y<<std::endl;
	return numberPoints / (AABBSize.x * AABBSize.y);
}

bool& PPCX::PointCloud::getVisible() {
	return visible;
}
