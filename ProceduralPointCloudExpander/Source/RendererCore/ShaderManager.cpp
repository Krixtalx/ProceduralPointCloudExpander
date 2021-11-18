//
// Created by Niskp on 02/10/2021.
//

#include "stdafx.h"
#include "ShaderManager.h"

PAG::ShaderManager *PAG::ShaderManager::instancia = nullptr;

/**
 * Consulta del objeto único de la clase
 * @return Puntero al ShaderManager
 */
PAG::ShaderManager *PAG::ShaderManager::getInstancia() {
	if (!instancia) {
		instancia = new ShaderManager;
	}
	return instancia;
}

/**
 * Destructor. Elimina todos los shaders y shaderPrograms (que estén en los mapas) del contexto OpenGL
 */
PAG::ShaderManager::~ShaderManager() {
	for (const auto &shader: shaders) {
		delete shader.second;
	}

	for (const auto &shaderProgram: shaderPrograms) {
		delete shaderProgram.second;
	}
}

/**
 * Añade un nuevo shader al mapa shaders.
 * @param nombreShader nombre con el que se incluirá en el mapa
 * @param tipoShader
 * @param ruta en la que se encuentra el código fuente
 */
void PAG::ShaderManager::nuevoShader(const std::string &nombreShader, GLenum tipoShader, const std::string &ruta) {
	auto nuevoShader = new Shader(nombreShader, tipoShader, ruta);
	shaders.insert(std::make_pair(nombreShader, nuevoShader));
}

/**
 * Añade un nuevo shaderProgram al mapa shaderPrograms
 * @param nombreSP nombre con el que se incluirá en el mapa
 */
void PAG::ShaderManager::nuevoShaderProgram(const std::string &nombreSP) {
	auto nuevoSP = new ShaderProgram();
	shaderPrograms.insert(std::make_pair(nombreSP, nuevoSP));
}

/**
 * Añade el shader al shader program indicado
 * @param nombreShader a añadir
 * @param nombreSP al que se le añadirá
 */
void PAG::ShaderManager::addShaderToSP(const std::string &nombreShader, const std::string &nombreSP) {
	const auto SP = shaderPrograms.find(nombreSP);
	if (SP != shaderPrograms.end()) {
		SP->second->addShader(shaders.find(nombreShader)->second);
	} else {
		throw std::runtime_error(
				"[ShaderManager]: No se ha encontrado ningun shader program con el nombre " + nombreSP);
	}
}

/**
 * Activa el shader program indicado
 * @param nombreSP a activar
 */
void PAG::ShaderManager::activarSP(const std::string &nombreSP) {
	const auto SP = shaderPrograms.find(nombreSP);
	if (SP != shaderPrograms.end()) {
		SP->second->activateShaderProgram();
	} else {
		throw std::runtime_error(
				"[ShaderManager]: No se ha encontrado ningun shader program con el nombre " + nombreSP);
	}
}

/**
 * Método para establecer una variable uniform dentro de un Shader Program
 * @param nombreSP nombre del ShaderProgram en el que establecer el uniform
 * @param variable nombre de la variable a establecer
 */
void PAG::ShaderManager::setUniform(const std::string &nombreSP, const std::string &variable, glm::mat4 matrizMVP) {
	const auto SP = shaderPrograms.find(nombreSP);
	if (SP != shaderPrograms.end()) {
		const GLint location = glGetUniformLocation(SP->second->getIdSP(), variable.c_str());
		if (location >= 0) {
			glUniformMatrix4fv(location, 1, false, glm::value_ptr(matrizMVP));
		} else
			throw std::runtime_error(
					"[ShaderManager]: No se ha encontrado ninguna variable con el nombre " + variable +
					" en el shaderProgram " + nombreSP);
	} else {
		throw std::runtime_error(
				"[ShaderManager]: No se ha encontrado ningun shader program con el nombre " + nombreSP);
	}

}

/**
 * Método para establecer una variable uniform dentro de un Shader Program
 * @param nombreSP nombre del ShaderProgram en el que establecer el uniform
 * @param variable nombre de la variable a establecer
 */
void PAG::ShaderManager::setUniform(const std::string &nombreSP, const std::string &variable, glm::vec3 vec) {
	const auto SP = shaderPrograms.find(nombreSP);
	if (SP != shaderPrograms.end()) {
		const GLint location = glGetUniformLocation(SP->second->getIdSP(), variable.c_str());
		if (location >= 0) {
			glUniform3fv(location, 1, glm::value_ptr(vec));
		} else
			throw std::runtime_error(
					"[ShaderManager]: No se ha encontrado ninguna variable con el nombre " + variable +
					" en el shaderProgram " + nombreSP);
	} else {
		throw std::runtime_error(
				"[ShaderManager]: No se ha encontrado ningun shader program con el nombre " + nombreSP);
	}
}

/**
 * Método que permite activar una subrutina en determinado ShaderProgram
 * @param nombreSP nombre del ShaderProgram al que se le activará la subrutina
 * @param tipoShader en que shader se encuentra la subrutina (GL_VERTEX_SHADER o GL_FRAGMENT_SHADER)
 * @param nombreSubrutina nombre de la subrutina a activar
 */
void PAG::ShaderManager::activarSubrutina(const std::string &nombreSP, GLenum tipoShader,
                                          const std::string &nombreSubrutina) {
	const auto SP = shaderPrograms.find(nombreSP);
	if (SP != shaderPrograms.end()) {
		const GLuint location = glGetSubroutineIndex(SP->second->getIdSP(), tipoShader, nombreSubrutina.c_str());
		glUniformSubroutinesuiv(tipoShader, 1, &location);
	} else {
		throw std::runtime_error(
				"[ShaderManager]: No se ha encontrado ningun shader program con el nombre " + nombreSP);
	}
}


