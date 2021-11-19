//
// Created by Niskp on 15/09/2021.
//

#include "stdafx.h"
#include "Renderer.h"
#include "ShaderManager.h"
#include "Utilities/PlyLoader.h"


PPCX::Renderer *PPCX::Renderer::instancia = nullptr;

/**
 * Constructor por defecto
 */
PPCX::Renderer::Renderer() {
	try {
		PPCX::ShaderManager::getInstancia()->nuevoShader("VertexShader", GL_VERTEX_SHADER, "Source/Shaders/VertexShader.glsl");
		PPCX::ShaderManager::getInstancia()->nuevoShader("FragmentShader", GL_FRAGMENT_SHADER, "Source/Shaders/FragmentShader.glsl");
		PPCX::ShaderManager::getInstancia()->nuevoShaderProgram("DefaultSP");
		PPCX::ShaderManager::getInstancia()->addShaderToSP("VertexShader", "DefaultSP");
		PPCX::ShaderManager::getInstancia()->addShaderToSP("FragmentShader", "DefaultSP");
	} catch (std::runtime_error &e) {
		throw;
	}
	//modelos.push_back(PlyLoader::cargarModelo("NubeDensa"));
}

/**
 * Destructor
 */
PPCX::Renderer::~Renderer() {
	for (const auto modelo: modelos) {
		delete modelo;
	}
}

/**
 * Consulta del objeto único de la clase
 * @return Puntero al Renderer
 */
PPCX::Renderer *PPCX::Renderer::getInstancia() {
	if (!instancia) {
		try {
			instancia = new Renderer;
		} catch (std::runtime_error &e) {
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
void PPCX::Renderer::inicializaOpenGL() {
	activarUtilidadGL(GL_DEPTH_TEST);
	activarUtilidadGL(GL_MULTISAMPLE);
	activarUtilidadGL(GL_DEBUG_OUTPUT);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	actualizarColorFondo();
}

/**
 * Método para hacer el refresco de la escena
 */
void PPCX::Renderer::refrescar() const {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const glm::mat4 matrizMVP = camara.matrizMVP();
	for (const auto modelo: modelos) {
		if (modelo)
			try {
				modelo->dibujarModelo(matrizMVP);
			} catch (std::runtime_error &e) {
				throw;
			}
	}
}

/**
 * Método para cambiar el colorSeleccionado de fondo de la escena
 * @param red
 * @param green
 * @param blue
 * @param alpha
 */
void PPCX::Renderer::setColorFondo(glm::vec3 color) {
	this->colorFondo = color;
	actualizarColorFondo();
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
const GLubyte *PPCX::Renderer::getPropiedadGL(GLenum propiedad) {
	return glGetString(propiedad);
}

/**
 * Activa la utilidad de OpenGL indicada
 * @param utility
 */
void PPCX::Renderer::activarUtilidadGL(GLenum utility) {
	glEnable(utility);
}

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
 * Limpia el buffer OpenGL indicado en la mascara
 * @param mascara
 */
void PPCX::Renderer::limpiarGL(GLbitfield mascara) {
	glClear(mascara);
}

void PPCX::Renderer::cargaModelo(const std::string& path)
{
	modelos.push_back(PlyLoader::cargarModelo(path));
}

/**
 * ------------------- Getters y Setters ------------------------------------
 */

PPCX::Camara &PPCX::Renderer::getCamara() {
	return camara;
}

glm::vec3& PPCX::Renderer::getColorFondo()
{
	return colorFondo;
}

