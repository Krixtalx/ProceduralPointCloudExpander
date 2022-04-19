//
// Created by Niskp on 02/10/2021.
//

#include "stdafx.h"
#include "ShaderManager.h"

#include "ComputeShader.h"

PPCX::ShaderManager* PPCX::ShaderManager::instancia = nullptr;

/**
 * Consulta del objeto único de la clase
 * @return Puntero al ShaderManager
 */
PPCX::ShaderManager* PPCX::ShaderManager::getInstancia() {
	if (!instancia) {
		instancia = new ShaderManager;
	}
	return instancia;
}

/**
 * Destructor. Elimina todos los shaders y shaderPrograms (que estén en los mapas) del contexto OpenGL
 */
PPCX::ShaderManager::~ShaderManager() {
	for (const auto& shader : shaders) {
		delete shader.second;
	}

	for (const auto& shaderProgram : shaderPrograms) {
		delete shaderProgram.second;
	}
}

/**
 * Añade un nuevo shader al mapa shaders.
 * @param nombreShader nombre con el que se incluirá en el mapa
 * @param tipoShader
 * @param ruta en la que se encuentra el código fuente
 */
void PPCX::ShaderManager::nuevoShader(const std::string& nombreShader, const GLenum tipoShader, const std::string& ruta) {
	auto nuevoShader = new Shader(nombreShader, tipoShader, ruta);
	shaders.insert(std::make_pair(nombreShader, nuevoShader));
}

/**
 * Añade un nuevo shaderProgram al mapa shaderPrograms
 * @param nombreSP nombre con el que se incluirá en el mapa
 */
void PPCX::ShaderManager::nuevoShaderProgram(const std::string& nombreSP) {
	auto nuevoSP = new ShaderProgram();
	shaderPrograms.insert(std::make_pair(nombreSP, nuevoSP));
}

/**
 * Añade el shader al shader program indicado
 * @param nombreShader a añadir
 * @param nombreSP al que se le añadirá
 */
void PPCX::ShaderManager::addShaderToSP(const std::string& nombreShader, const std::string& nombreSP) {
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
void PPCX::ShaderManager::activarSP(const std::string& nombreSP) {
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
void PPCX::ShaderManager::setUniform(const std::string& nombreSP, const std::string& variable, glm::mat4 matriz) {
	const auto SP = shaderPrograms.find(nombreSP);
	if (SP != shaderPrograms.end()) {
		const GLint location = glGetUniformLocation(SP->second->getIdSP(), variable.c_str());
		if (location >= 0) {
			glUniformMatrix4fv(location, 1, false, value_ptr(matriz));
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
void PPCX::ShaderManager::setUniform(const std::string& nombreSP, const std::string& variable, glm::vec3 vec) {
	const auto SP = shaderPrograms.find(nombreSP);
	if (SP != shaderPrograms.end()) {
		const GLint location = glGetUniformLocation(SP->second->getIdSP(), variable.c_str());
		if (location >= 0) {
			glUniform3fv(location, 1, value_ptr(vec));
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
void PPCX::ShaderManager::setUniform(const std::string& nombreSP, const std::string& variable, GLuint valor) {
	const auto SP = shaderPrograms.find(nombreSP);
	if (SP != shaderPrograms.end()) {
		const GLint location = glGetUniformLocation(SP->second->getIdSP(), variable.c_str());
		if (location >= 0) {
			glUniform1ui(location, valor);
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
void PPCX::ShaderManager::setUniform(const std::string& nombreSP, const std::string& variable, GLint valor) {
	const auto SP = shaderPrograms.find(nombreSP);
	if (SP != shaderPrograms.end()) {
		const GLint location = glGetUniformLocation(SP->second->getIdSP(), variable.c_str());
		if (location >= 0) {
			glUniform1i(location, valor);
		} else
			throw std::runtime_error(
				"[ShaderManager]: No se ha encontrado ninguna variable con el nombre " + variable +
				" en el shaderProgram " + nombreSP);
	} else {
		throw std::runtime_error(
			"[ShaderManager]: No se ha encontrado ningun shader program con el nombre " + nombreSP);
	}
}

void PPCX::ShaderManager::setUniform(const std::string& nombreSP, const std::string& variable, float valor) {
	const auto SP = shaderPrograms.find(nombreSP);
	if (SP != shaderPrograms.end()) {
		const GLint location = glGetUniformLocation(SP->second->getIdSP(), variable.c_str());
		if (location >= 0) {
			glUniform1f(location, valor);
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
void PPCX::ShaderManager::activarSubrutina(const std::string& nombreSP, GLenum tipoShader,
										   const std::string& nombreSubrutina) {
	const auto SP = shaderPrograms.find(nombreSP);
	if (SP != shaderPrograms.end()) {
		const GLuint location = glGetSubroutineIndex(SP->second->getIdSP(), tipoShader, nombreSubrutina.c_str());
		glUniformSubroutinesuiv(tipoShader, 1, &location);
	} else {
		throw std::runtime_error(
			"[ShaderManager]: No se ha encontrado ningun shader program con el nombre " + nombreSP);
	}
}

/**
 * Método que permite activar varias subrutinas en determinado ShaderProgram
 * @param nombreSP nombre del ShaderProgram al que se le activará la subrutina
 * @param tipoShader en que shader se encuentra la subrutina (GL_VERTEX_SHADER o GL_FRAGMENT_SHADER)
 * @param nombreUniform nombres de los uniforms a activar
 * @param nombreSubrutina nombres de la subrutinas a activar
 */
void PPCX::ShaderManager::activarMultiplesSubrutinas(const std::string& nombreSP, const GLenum tipoShader,
													 const std::vector<std::string>& nombreUniform,
													 const std::vector<std::string>& nombreSubrutina) {
	const auto SP = shaderPrograms.find(nombreSP);
	if (SP != shaderPrograms.end()) {
		if (nombreUniform.size() != nombreSubrutina.size())
			throw std::runtime_error(
				"[ShaderManager::MultiplesSubrutinas]: No se ha pasado el mismo nº de uniforms que de subrutinas");

		GLint tam = 0;
		glGetProgramStageiv(SP->second->getIdSP(), tipoShader,
							GL_ACTIVE_SUBROUTINE_UNIFORMS, &tam);

		if (nombreUniform.size() != tam)
			throw std::runtime_error(
				"[ShaderManager::MultiplesSubrutinas]: No coincide el nº de uniforms pasados con el nº de uniforms en el shader");

		const auto valores = new GLuint[tam];
		for (int i = 0; i < tam; ++i) {
			const GLint posUniform = glGetSubroutineUniformLocation(SP->second->getIdSP(), tipoShader,
																	nombreUniform[i].c_str());
			if (posUniform == -1) {
				throw std::runtime_error("[ShaderManager::MultiplesSubrutinas]: No se ha podido localizar el uniform " +
										 nombreUniform[i] + " en " + nombreSP);
			}
			const GLuint location = glGetSubroutineIndex(SP->second->getIdSP(), tipoShader, nombreSubrutina[i].c_str());
			if (location == GL_INVALID_INDEX) {
				throw std::runtime_error(
					"[ShaderManager::MultiplesSubrutinas]: No se ha podido localizar la implementación " +
					nombreSubrutina[i] + " de " + nombreUniform[i] + " en " + nombreSP);
			}
			valores[posUniform] = location;
		}

		glUniformSubroutinesuiv(tipoShader, tam, valores);
		delete[] valores;
	} else {
		throw std::runtime_error(
			"[ShaderManager]: No se ha encontrado ningun shader program con el nombre " + nombreSP);
	}
}

PPCX::ComputeShader* PPCX::ShaderManager::getComputeShader(const std::string& name) {
	const auto shader = shaders.find(name);
	Shader* s(shader->second);
	if (shader == shaders.end() || s->getTipoShader() != GL_COMPUTE_SHADER) {
		throw std::runtime_error("[ShaderManager]: No se ha encontrado ningun ComputeShader con el nombre " + name);
	}

	return dynamic_cast<ComputeShader*>(s);
}
