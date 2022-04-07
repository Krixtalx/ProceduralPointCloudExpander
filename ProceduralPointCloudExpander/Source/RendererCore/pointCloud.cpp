#include "stdafx.h"
#include "PointCloud.h"
#include <utility>
#include "ShaderManager.h"
#include "GeometryUtils/AABB.h"
#include "morton.h"


PointCloud::PointCloud(std::string shaderProgram, const vec3& pos, const vec3& rot, const vec3& scale) : Model(std::move(shaderProgram), pos, rot, scale) {}

/**
 * Constructor parametrizado
 * @param shaderProgram que se usará para renderizar el modelo
 * @param points puntos de la nube
 * @param aabb
 * @param pos Posicion inicial de la nube de puntos
 */
PointCloud::PointCloud(std::string shaderProgram, const std::vector<PointModel>& points, const AABB& aabb, const vec3& pos, const
                       vec3& rot, const vec3& scale) :
	Model(std::move(shaderProgram), pos, rot, scale), aabb(aabb), needUpdating(true) {
	newPoints(points);
}

/**
 * Constructor copia. Copia el numVertices y el shaderProgram y realiza una nueva instanciacion de los vbos e ibos
 * @param orig nube de puntos original
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

void PointCloud::newPoint(const PointModel & point) {
	vbo.push_back(point);
	aabb.update(point._point);
	needUpdating = true;
	optimized = false;
}

void PointCloud::newPoints(const std::vector<PointModel>&points) {
	vbo.clear();
	vbo.resize(points.size());
	std::copy(points.begin(), points.end(), vbo.begin());
	needUpdating = true;
	optimized = false;
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
void PointCloud::newIBO(const std::vector<GLuint>&data, const GLenum freqAct) {
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
void PointCloud::drawModel(const mat4& MVPMatrix) {
	if (visible) {
		if (needUpdating)
			updateCloud();
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

vec3 PointCloud::getRandomPointColor() {
	return vbo[rand() % vbo.size()].getRGBVec3();
}

bool compareFunction(const PointModel & a, const PointModel & b) {
	const uint_fast64_t aVal = libmorton::morton3D_64_encode(a._point.x * 10000, a._point.y * 10000, a._point.z * 10000);
	const uint_fast64_t bVal = libmorton::morton3D_64_encode(b._point.x * 10000, b._point.y * 10000, b._point.z * 10000);
	return aVal < bVal;
}

void PointCloud::optimize() {
	if (!optimized) {
		std::sort(vbo.begin(), vbo.end(), compareFunction);
		std::vector<PointModel> newVbo(vbo.size());
		const auto size = static_cast<unsigned>(vbo.size() / 128 + .5f);
		std::vector<unsigned> batchOrder(size);
		std::iota(batchOrder.begin(), batchOrder.end(), 0);
		const unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
		std::shuffle(batchOrder.begin(), batchOrder.end(), std::default_random_engine(seed));

		for (int i = 0; i < size; ++i) {
			std::copy_n(vbo.begin() + 128 * batchOrder[i], 128, newVbo.begin() + 128 * i);
		}
		vbo = newVbo;
		newVBO(GL_STATIC_DRAW);
		optimized = true;
	}
}

