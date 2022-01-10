//
// Created by Niskp on 15/09/2021.
//

#include "stdafx.h"
#include "Renderer.h"
#include "ShaderManager.h"
#include "Utilities/PlyLoader.h"


PPCX::Renderer* PPCX::Renderer::instancia = nullptr;

/**
 * Constructor por defecto
 */
PPCX::Renderer::Renderer() {
	try {
		ShaderManager::getInstancia()->nuevoShader("VertexShader", GL_VERTEX_SHADER, "Source/Shaders/VertexShader.glsl");
		ShaderManager::getInstancia()->nuevoShader("FragmentShader", GL_FRAGMENT_SHADER, "Source/Shaders/FragmentShader.glsl");
		ShaderManager::getInstancia()->nuevoShaderProgram("DefaultSP");
		ShaderManager::getInstancia()->addShaderToSP("VertexShader", "DefaultSP");
		ShaderManager::getInstancia()->addShaderToSP("FragmentShader", "DefaultSP");
	} catch (std::runtime_error& e) {
		throw;
	}

}

/**
 * Destructor
 */
PPCX::Renderer::~Renderer() {}


/**
 * Consulta del objeto único de la clase
 * @return Puntero al Renderer
 */
PPCX::Renderer* PPCX::Renderer::getInstancia() {
	if (!instancia) {
		try {
			instancia = new Renderer;
		} catch (std::runtime_error& e) {
			throw;
		}
	}
	return instancia;
}

/**
 * Inicializa los parámetros globales de OpenGL.
 * Establece un color de fondo inicial
 * Activa el ZBuffer.
 * Activa el Antialiasing MultiSampling (MSAA)
 */
void PPCX::Renderer::inicializaOpenGL() const {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_CULL_FACE);
	glPointSize(pointSize);
	actualizarColorFondo();
}

/**
 * Método para hacer el refresco de la escena
 */
void PPCX::Renderer::refrescar() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const mat4 matrizMVP = camara.matrizMVP();
	procGenerator.drawClouds(matrizMVP);
}

/**
 * Método que actualiza el color de fondo de la escena al indicado por los atributos
 */
void PPCX::Renderer::actualizarColorFondo() const {
	glClearColor(colorFondo[0], colorFondo[1], colorFondo[2], 1);
}

/**
 * Método que nos permite obtener las propiedades del contexto OpenGL construido
 * @param propiedad a obtener
 * @return GLubyte con la propiedad requerida
 */
const GLubyte* PPCX::Renderer::getPropiedadGL(GLenum propiedad) {
	return glGetString(propiedad);
}

void PPCX::Renderer::cargaModelo(const std::string& path, const bool& newScene) {
	PointCloud* pCloud = PlyLoader::loadPointCloud(path);
	if (pCloud) {
		procGenerator.newPointCloud(pCloud, newScene);
		vec3 pos = pCloud->getAABB().max();
		camara.increaseZFar(distance(pos, pCloud->getAABB().center()));
		pos = rotate(glm::pi<float>() / 6.0f, vec3(1.0f, .0f, 1.0f)) * vec4(pos, 0.0f);
		camara.setPosicion(pos);
		camara.setPuntoMira(pCloud->getAABB().center());
		camara.setSpeedMultiplier((pCloud->getAABB().size().x + pCloud->getAABB().size().y) * 0.02f);

	} else {
		std::cerr << "Point cloud load has failed" << std::endl;
	}
}

/**
 * ------------------- Getters y Setters ------------------------------------
 */

 /**
  * Cambia el tamaño del viewport al indicado
  * @param x punto de referencia
  * @param y punto de referencia
  * @param width ancho de la ventana
  * @param height alto de la ventana
  */
void PPCX::Renderer::setViewport(GLint x, GLint y, GLsizei width, GLsizei height) {
	glViewport(x, y, width, height);
	camara.setAlto(height);
	camara.setAncho(width);
}

/**
 * Método para cambiar el colorSeleccionado de fondo de la escena
 */
void PPCX::Renderer::setColorFondo(vec3 color) {
	this->colorFondo = color;
	actualizarColorFondo();
}

PPCX::Camara& PPCX::Renderer::getCamara() {
	return camara;
}

vec3& PPCX::Renderer::getColorFondo() {
	return colorFondo;
}

float PPCX::Renderer::getPointSize() const {
	return pointSize;
}

void PPCX::Renderer::setPointSize(float pointS) {
	pointSize = pointS;
	glPointSize(pointSize);
}

ProceduralGenerator* PPCX::Renderer::getProceduralGenerator() {
	return &procGenerator;
}