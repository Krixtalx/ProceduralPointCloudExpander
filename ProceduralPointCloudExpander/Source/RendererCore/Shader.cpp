//
// Created by Niskp on 01/10/2021.
//

#include "stdafx.h"
#include "Shader.h"


/**
 * Constructor parametrizado. Realiza todo el proceso necesario de carga y compilación del shader
 * @param tipoShader
 * @param ruta donde se encuentra el código GLSL del shader
 * @throw runtime_error en caso de que ocurra algún error durante la carga o compilación
 */
PAG::Shader::Shader(std::string nombreShader, GLenum tipoShader, const std::string &ruta) : nombreShader(std::move(
		nombreShader)), tipoShader(tipoShader) {
	idShader = glCreateShader(tipoShader);
	if (idShader == 0) {
		// Ha ocurrido un error al intentar crear el shader
		throw std::runtime_error("[Shader]: Error desconocido al intentar construir el shader.");
	} else {
		try {
			cargaShader(ruta);
			compilaShader();
		} catch (std::runtime_error &e) {
			throw e;
		}
	}
}

/**
 * Destructor.
 */
PAG::Shader::~Shader() {
	glDeleteShader(idShader);
}

/**
 * Carga el código del shader
 * @param ruta donde se encuentra el código GLSL del shader
 * @throw runtime_error Si no se puede abrir el fichero
 */
void PAG::Shader::cargaShader(const std::string &ruta) const {
	std::ifstream archivoShader;
	archivoShader.open(ruta, std::ifstream::in);

	if (!archivoShader.is_open()) {
		// Error abriendo el archivo
		throw std::runtime_error("[Shader::cargaShader]: Ha ocurrido un error al intentar abrir el fichero " + ruta);
	}

	std::stringstream streamShader;
	streamShader << archivoShader.rdbuf();
	std::string codigoFuenteShader = streamShader.str();

	archivoShader.close();

	const GLchar *codigoFuenteFormatoC = codigoFuenteShader.c_str();
	glShaderSource(idShader, 1, &codigoFuenteFormatoC, nullptr);
}

/**
 * Comprueba si se ha producido algún error en la compilación del shader
 * @throw runtime_error en caso de que haya algún error de compilación
 */
void PAG::Shader::compruebaErroresShader() const {
	GLint resultadoCompilacion;
	glGetShaderiv(idShader, GL_COMPILE_STATUS, &resultadoCompilacion);

	if (resultadoCompilacion == GL_FALSE) {
		// Ha habido un error en la compilación.
		// Para saber qué ha pasado, tenemos que recuperar el mensaje de error de OpenGL
		GLint tamMsj = 0;
		std::string mensaje;
		glGetShaderiv(idShader, GL_INFO_LOG_LENGTH, &tamMsj);
		if (tamMsj > 0) {
			auto *mensajeFormatoC = new GLchar[tamMsj];
			GLint datosEscritos = 0;
			glGetShaderInfoLog(idShader, tamMsj, &datosEscritos, mensajeFormatoC);
			mensaje.assign(mensajeFormatoC);
			mensaje = "[Shader::compruebaErroresShader]: " + nombreShader + "->" + mensaje;
			delete[] mensajeFormatoC;
			throw std::runtime_error(mensaje);
		}
	}
}

/**
 * Realiza la compilación del shader y después lanza la comprobación de errores
 * @throw runtime_error en caso de que haya algún error de compilación
 */
void PAG::Shader::compilaShader() {
	glCompileShader(idShader);
	try {
		compruebaErroresShader();
	} catch (std::runtime_error &e) {
		throw e;
	}
}


//---------Getters y setters----------------------------------------

GLuint PAG::Shader::getShaderId() const {
	return idShader;
}

GLenum PAG::Shader::getTipoShader() const {
	return tipoShader;
}

