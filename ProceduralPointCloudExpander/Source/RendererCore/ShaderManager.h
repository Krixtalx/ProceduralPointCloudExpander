//
// Created by Niskp on 02/10/2021.
//

#ifndef PAG_SHADERMANAGER_H
#define PAG_SHADERMANAGER_H

#include "ShaderProgram.h"

namespace PPCX {
	class ShaderManager {
	private:
		static ShaderManager *instancia;

		ShaderManager() = default;

		std::map<std::string, PPCX::ShaderProgram*> shaderPrograms;
		std::map<std::string, PPCX::Shader*> shaders;
	public:
		static ShaderManager *getInstancia();

		ShaderManager(const ShaderManager &orig) = delete;

		~ShaderManager();

		void nuevoShader(const std::string &nombreShader, GLenum tipoShader, const std::string &ruta);

		void nuevoShaderProgram(const std::string &nombreSP);

		void addShaderToSP(const std::string &nombreShader, const std::string &nombreSP);

		void activarSP(const std::string &nombreSP);

		void setUniform(const std::string &nombreSP, const std::string &variable, glm::mat4 matrizMVP);

		void setUniform(const std::string &nombreSP, const std::string &variable, glm::vec3 vec);

		void activarSubrutina(const std::string &nombreSP, GLenum tipoShader, const std::string &nombreSubrutina);
	};
}


#endif //PAG_SHADERMANAGER_H
