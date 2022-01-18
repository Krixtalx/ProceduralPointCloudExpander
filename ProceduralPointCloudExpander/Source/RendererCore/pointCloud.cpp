#include "stdafx.h"
#include "PointCloud.h"
#include <utility>
#include "ShaderManager.h"
#include "GeometryUtils/AABB.h"


/**
 * Constructor parametrizado
 * @param shaderProgram que se usará para renderizar el modelo
 * @param pos Posicion inicial de la nube de puntos
 */
PointCloud::PointCloud(std::string shaderProgram, const std::vector<PointModel>& points, const AABB& aabb, const
					   vec3& pos) :
	Model(shaderProgram, pos), aabb(aabb), needUpdating(true) {
	newPoints(points);
}

/**
 * Constructor copia. Copia el numVertices y el shaderProgram y realiza una nueva instanciacion de los vbos e ibos
 * @param orig
 */
PointCloud::PointCloud(PointCloud& orig) : Model(orig), vbo(orig.vbo) {
	//Creamos nuestro VAO
	glGenVertexArrays(1, &idVAO);
	glBindVertexArray(idVAO);

	newVBO(GL_STATIC_DRAW);
}

/**
 * Destructor. Libera todos los recursos reservados a OpenGL.
 */
PointCloud::~PointCloud() = default;

void PointCloud::newPoint(const PointModel& point) {
	vbo.push_back(point);
	aabb.update(point._point);
}

void PointCloud::newPoints(const std::vector<PointModel>& points) {
	vbo.clear();
	vbo.resize(points.size());
	std::copy(points.begin(), points.end(), vbo.begin());
}

void PointCloud::updateCloud() {
	if (idVAO == UINT_MAX) {
		glGenVertexArrays(1, &idVAO);
		glBindVertexArray(idVAO);
	}
	newVBO(GL_STATIC_DRAW);
	std::vector<unsigned> ibo;
	ibo.resize(vbo.size());
	std::iota(ibo.begin(), ibo.end(), 0);
	newIBO(ibo, GL_STATIC_DRAW);
	needUpdating = false;
}

/**
 * Instancia un VBO en el contexto OpenGL y lo guarda en vbos
 * @param datos a instanciar
 * @param freqAct GLenum que indica con que frecuencia se van a modificar los vertices. GL_STATIC_DRAW siempre por ahora
 */
void PointCloud::newVBO(GLenum freqAct) {
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
 * @param data a instanciar
 * @param freqAct GLenum que indica con que frecuencia se van a modificar los vertices. GL_STATIC_DRAW siempre por ahora
 */
void PointCloud::newIBO(const std::vector<GLuint> data, const GLenum freqAct) {
	//Si hay un buffer de este tipo instanciado, lo eliminamos
	if (idIBO != UINT_MAX) {
		glDeleteBuffers(1, &idIBO);
	}

	glBindVertexArray(idVAO);
	glGenBuffers(1, &idIBO);
	glBindBuffer(GL_ARRAY_BUFFER, idIBO);
	glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(GLuint), data.data(), freqAct);
}


/**
 * Función a la que se llama cuando se debe de dibujar el modelo
 */
void PointCloud::drawModel(const mat4 MVPMatrix) const {
	if (visible) {
		try {
			//MVPMatrix = MVPMatrix * translate(pos);
			PPCX::ShaderManager::getInstancia()->activarSP(shaderProgram);
			PPCX::ShaderManager::getInstancia()->setUniform(this->shaderProgram, "matrizMVP", MVPMatrix);

			glBindVertexArray(idVAO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idIBO);
			glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
			glDrawElements(GL_POINTS, vbo.size(), GL_UNSIGNED_INT, nullptr); //Al ser una nube de puntos vbo.size = ibo.size
		} catch (std::runtime_error& e) {
			throw;
		}
	}
}

std::vector<PointModel>& PointCloud::getPoints() {
	return vbo;
}

unsigned PointCloud::getNumberOfPoints() const {
	return vbo.size();
}

const AABB& PointCloud::getAABB() {
	return aabb;
}

float PointCloud::getDensity() const {
	const int numberPoints = vbo.size();
	const vec3 AABBSize = aabb.size();
	return numberPoints / (AABBSize.x * AABBSize.y);
}