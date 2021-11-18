//
// Created by Niskp on 01/10/2021.
//

#include "stdafx.h"
#include "ShaderProgram.h"

/**
 * Constructor por defecto
 */
PAG::ShaderProgram::ShaderProgram() {
	idSP = glCreateProgram();
}

PAG::ShaderProgram::ShaderProgram(std::vector<Shader *> shaders) : shaders(std::move(shaders)) {
	idSP = glCreateProgram();
	for (const auto &shader: this->shaders) {
		glAttachShader(idSP, shader->getShaderId());
	}
}

/**
 * Destructor.
 */
PAG::ShaderProgram::~ShaderProgram() {
	glDeleteProgram(idSP);
}

/**
 * Añade un nuevo Shader a este Shader Program
 * @param shader Puntero al shader a añadir
 * @throw runtime_error en caso de que el puntero sea nulo
 */
void PAG::ShaderProgram::addShader(PAG::Shader *shader) {
	if (shader != nullptr) {
		shaders.push_back(shader);
		glAttachShader(idSP, shader->getShaderId());
		linked = false;
	} else {
		throw std::runtime_error("[ShaderProgram]: El shader que se ha pasado como parámetro es un puntero nulo");
	}
}

/**
 * Enlaza el ShaderProgram si no esta enlazado y lo establece como activo
 */
void PAG::ShaderProgram::activateShaderProgram() {
	try {
		if (!linked) {
			glLinkProgram(idSP);
			compruebaErroresSP();
			linked = true;
		}
		glUseProgram(idSP);
	} catch (std::runtime_error &e) {
		throw;
	}
}

/**
* Comprueba si se ha producido algún error en el enlazado del Shader Program
* @throw runtime_error en caso de que haya algún error de compilación
*/
void PAG::ShaderProgram::compruebaErroresSP() const {

	GLint resultadoCompilacion;
	glGetProgramiv(idSP, GL_LINK_STATUS, &resultadoCompilacion);

	if (resultadoCompilacion == GL_FALSE) {
		// Ha habido un error en la compilación.
		// Para saber qué ha pasado, tenemos que recuperar el mensaje de error de OpenGL
		GLint tamMsj = 0;
		std::string mensaje;
		glGetProgramiv(idSP, GL_INFO_LOG_LENGTH, &tamMsj);
		if (tamMsj > 0) {
			auto *mensajeFormatoC = new GLchar[tamMsj];
			GLint datosEscritos = 0;
			glGetShaderInfoLog(idSP, tamMsj, &datosEscritos, mensajeFormatoC);
			mensaje.assign(mensajeFormatoC);
			mensaje = "[ShaderProgram]:" + mensaje;
			delete[] mensajeFormatoC;
			throw std::runtime_error(mensaje);
		}
	}
}

//-------------Getters y setters-----------------------
GLuint PAG::ShaderProgram::getIdSP() const {
	return idSP;
}