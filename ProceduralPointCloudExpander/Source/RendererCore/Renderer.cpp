//
// Created by Niskp on 15/09/2021.
//

#include "stdafx.h"
#include "Renderer.h"
#include "ShaderManager.h"
#include "Utilities/Loader.h"
#include "RendererCore/ModelManager.h"
#include <RendererCore/InstancedPointCloud.h>

#include "Material.h"
#include "MaterialManager.h"
#include "TriangleMesh.h"


PPCX::Renderer* PPCX::Renderer::instancia = nullptr;

/**
 * Constructor por defecto
 */
PPCX::Renderer::Renderer() {
	try {
		ShaderManager::getInstancia()->nuevoShader("VertexShader", GL_VERTEX_SHADER, "Source/Shaders/VertexShader.glsl");
		ShaderManager::getInstancia()->nuevoShader("InstancingVertexShader", GL_VERTEX_SHADER, "Source/Shaders/InstancingVertexShader.glsl");
		ShaderManager::getInstancia()->nuevoShader("FragmentShader", GL_FRAGMENT_SHADER, "Source/Shaders/FragmentShader.glsl");
		ShaderManager::getInstancia()->nuevoShader("FullVertexShader", GL_VERTEX_SHADER, "Source/Shaders/pag-vs.glsl");
		ShaderManager::getInstancia()->nuevoShader("FullFragmentShader", GL_FRAGMENT_SHADER, "Source/Shaders/pag-fs.glsl");
		ShaderManager::getInstancia()->nuevoShaderProgram("DefaultSP");
		ShaderManager::getInstancia()->addShaderToSP("VertexShader", "DefaultSP");
		ShaderManager::getInstancia()->addShaderToSP("FragmentShader", "DefaultSP");
		ShaderManager::getInstancia()->nuevoShaderProgram("InstancingSP");
		ShaderManager::getInstancia()->addShaderToSP("InstancingVertexShader", "InstancingSP");
		ShaderManager::getInstancia()->addShaderToSP("FragmentShader", "InstancingSP");
		ShaderManager::getInstancia()->nuevoShaderProgram("TriangleMeshSP");
		ShaderManager::getInstancia()->addShaderToSP("FullVertexShader", "TriangleMeshSP");
		ShaderManager::getInstancia()->addShaderToSP("FullFragmentShader", "TriangleMeshSP");

		Loader::loadPointCloud("OliveTree", false);
		Loader::loadPointCloud("PineTree", false);
		/*PPCX::MaterialManager::getInstancia()->nuevoMaterial("Vaca",
													   new PPCX::Material({ 0.7, 0.15, 0.7 }, { 1, 1, 1 }, { 0.8, 0.8, 0.8 },
																	32, "spot_texture.png"));
		auto newTriangleMesh = new TriangleMesh("TriangleMeshSP", "vaca.obj");
		newTriangleMesh->setMaterial("Vaca");
		ModelManager::getInstance()->newModel("Test", newTriangleMesh);*/
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

	while (currentScreenshot < pendingScreenshots.size()) {
		pendingScreenshot(pendingScreenshots[currentScreenshot].first, pendingScreenshots[currentScreenshot].second);
		currentScreenshot++;
	}

	const mat4 matrizMVP = camara.matrizMVP();
	ModelManager::getInstance()->drawModels(matrizMVP);
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

void PPCX::Renderer::cargaModelo(const std::string& path, const bool& newScene, const unsigned& pointsPerVoxel) {
	Loader::loadPointCloud(path);
	try {
		const auto pCloud = dynamic_cast<PointCloud*>(ModelManager::getInstance()->getModel("Ground"));
		procGenerator.newPointCloud(pCloud, newScene, pointsPerVoxel);
		vec3 pos = pCloud->getAABB().max();
		const float dist = distance(pos, pCloud->getAABB().min());
		camara.increaseZFar(dist);
		pos = rotate(glm::pi<float>() / 6.0f, vec3(1.0f, .0f, 1.0f)) * vec4(pos, 0.0f);
		camara.setPosicion(pos);
		camara.setPuntoMira(pCloud->getAABB().center());
		camara.setSpeedMultiplier((pCloud->getAABB().size().x + pCloud->getAABB().size().y) * 0.02f);
		const float y = sqrt((pow(dist, 2) - pow(camara.aspecto(), 2) / 2));
		const float x = camara.aspecto() * y;
		camara.setOrthoPoints(glm::vec2(-x, -y), glm::vec2(x, y));

	} catch (std::runtime_error& e) {
		std::cerr << "[Renderer:cargaModelo]: " << e.what() << std::endl;
	}
}

void PPCX::Renderer::screenshot(const std::string& filename) {
	const GLuint ancho = camara.getAncho();
	const GLuint alto = camara.getAlto();

	// Creación de un FBO
	GLuint idFBO;
	glGenFramebuffers(1, &idFBO);

	// Activación del FBO
	glBindFramebuffer(GL_FRAMEBUFFER, idFBO);

	// Creación de una textura para utilizarla de buffer de color
	GLuint idTextura;
	glGenTextures(1, &idTextura);
	// Activación de la textura
	glBindTexture(GL_TEXTURE_2D, idTextura);
	// Configuración de la textura. Las dimensiones son (ancho, alto)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, ancho, alto, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	// Creación de un renderbuffer object para utilizarlo como buffer de profundidad
	GLuint idRBO;
	glGenRenderbuffers(1, &idRBO);
	// Activación del RBO
	glBindRenderbuffer(GL_RENDERBUFFER, idRBO);
	// Configuración del RBO. Las dimensiones son (ancho, alto)
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, ancho, alto);

	// Se vincula la textura y el RBO al FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, idTextura, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, idRBO);

	// Se comprueba que el FBO está listo para su uso
	const GLenum estado = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (estado != GL_FRAMEBUFFER_COMPLETE) {
		// Hay un error. Se genera una excepción
	}

	// Ahora se puede utilizar el FBO. Es MUY IMPORTANTE ajustar el tamaño del viewport para que
	// coincida con el del FBO
	glViewport(0, 0, ancho, alto);
	// Lanzar el proceso de rendering
	refrescar();
	// Finalizado el proceso de rendering, se recupera el contenido del FBO
	auto* pixeles = new GLubyte[ancho * alto * 4];
	auto* flipped = new GLubyte[ancho * alto * 4];

	glReadPixels(0, 0, ancho, alto, GL_RGBA, GL_UNSIGNED_BYTE, pixeles);

	for (unsigned j = 0; j < alto; j++) {
		for (unsigned i = 0; i < ancho; i++) {
			for (unsigned k = 0; k < 4; k++) {
				flipped[j * ancho * 4 + i * 4 + k] = pixeles[(alto - j - 1) * ancho * 4 + (i) * 4 + k];
			}
		}
	}
	// Se guarda la imagen en formato PNG
	static int i = 0;
	while (FILE* file = fopen(("Captures/" + filename + std::to_string(i) + ".png").c_str(), "r")) {
		fclose(file);
		i++;
	}
	unsigned error = lodepng_encode32_file(("Captures/" + filename + std::to_string(i) + ".png").c_str(), (unsigned char*)flipped, ancho, alto);
	delete[] pixeles;
	delete[] flipped;

	// VUELTA A LA NORMALIDAD
	glDeleteTextures(1, &idTextura);
	glDeleteRenderbuffers(1, &idRBO);
	glDeleteFramebuffers(1, &idFBO);
	// Se vuelve a activar el renderbuffer por defecto
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// Se vuelve a activar la textura por defecto
	glBindTexture(GL_TEXTURE_2D, 0);

	// Se vuelve a activar el framebuffer del gestor de ventanas
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	std::cout << "Screenshot taken" << std::endl;
}

void PPCX::Renderer::pendingScreenshot(const std::string& filename, const std::string& modelKey) const {
	const GLuint ancho = camara.getAncho();
	const GLuint alto = camara.getAlto();

	// Creación de un FBO
	GLuint idFBO;
	glGenFramebuffers(1, &idFBO);

	// Activación del FBO
	glBindFramebuffer(GL_FRAMEBUFFER, idFBO);

	// Creación de una textura para utilizarla de buffer de color
	GLuint idTextura;
	glGenTextures(1, &idTextura);
	// Activación de la textura
	glBindTexture(GL_TEXTURE_2D, idTextura);
	// Configuración de la textura. Las dimensiones son (ancho, alto)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, ancho, alto, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	// Creación de un renderbuffer object para utilizarlo como buffer de profundidad
	GLuint idRBO;
	glGenRenderbuffers(1, &idRBO);
	// Activación del RBO
	glBindRenderbuffer(GL_RENDERBUFFER, idRBO);
	// Configuración del RBO. Las dimensiones son (ancho, alto)
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, ancho, alto);

	// Se vincula la textura y el RBO al FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, idTextura, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, idRBO);

	// Se comprueba que el FBO está listo para su uso
	GLenum estado = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (estado != GL_FRAMEBUFFER_COMPLETE) {
		// Hay un error. Se genera una excepción
	}

	// Ahora se puede utilizar el FBO. Es MUY IMPORTANTE ajustar el tamaño del viewport para que
	// coincida con el del FBO
	glViewport(0, 0, ancho, alto);
	// Lanzar el proceso de rendering
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	const auto matrix = camara.matrizMVP();
	ModelManager::getInstance()->drawAndDeleteSingleModel(modelKey, matrix);
	// Finalizado el proceso de rendering, se recupera el contenido del FBO
	GLubyte* pixeles = nullptr;
	pixeles = new GLubyte[ancho * alto * 4];
	glReadPixels(0, 0, ancho, alto, GL_RGBA, GL_UNSIGNED_BYTE, pixeles);
	// Se guarda la imagen en formato PNG (habría que darle la vuelta antes)
	unsigned error = lodepng_encode32_file(("Captures/" + filename + ".png").c_str(), (unsigned char*)pixeles, ancho, alto);
	delete[] pixeles;

	// VUELTA A LA NORMALIDAD
	glDeleteTextures(1, &idTextura);
	glDeleteRenderbuffers(1, &idRBO);
	glDeleteFramebuffers(1, &idFBO);
	// Se vuelve a activar el renderbuffer por defecto
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// Se vuelve a activar la textura por defecto
	glBindTexture(GL_TEXTURE_2D, 0);

	// Se vuelve a activar el framebuffer del gestor de ventanas
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	std::cout << "Screenshot taken" << std::endl;
}

void PPCX::Renderer::addPendingScreenshot(const std::string& filename, const std::string& modelKey) {
	pendingScreenshots.emplace_back(filename, modelKey);
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
void PPCX::Renderer::setViewport(const GLint x, const GLint y, const GLsizei width, const GLsizei height) {
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