//
// Created by Niskp on 02/10/2021.
//

#ifndef PAG_SHADERMANAGER_H
#define PAG_SHADERMANAGER_H

#include "ShaderProgram.h"

namespace PPCX {
	class ComputeShader;

	class ShaderManager {
		static ShaderManager* instancia;

		ShaderManager() = default;

		std::map<std::string, ShaderProgram*> shaderPrograms;
		std::map<std::string, Shader*> shaders;
		std::map<std::string, GLuint> uniformsLocation;
	public:
		static ShaderManager* getInstancia();

		ShaderManager(const ShaderManager& orig) = delete;

		~ShaderManager();

		void nuevoShader(const std::string& nombreShader, GLenum tipoShader, const std::string& ruta);

		void nuevoShaderProgram(const std::string& nombreSP);

		void addShaderToSP(const std::string& nombreShader, const std::string& nombreSP);

		void activarSP(const std::string& nombreSP);

		void setUniform(const std::string& nombreSP, const std::string& variable, glm::mat4 matriz);

		void setUniform(const std::string& nombreSP, const std::string& variable, glm::vec3 vec);

		void setUniform(const std::string& nombreSP, const std::string& variable, GLuint valor);

		void setUniform(const std::string& nombreSP, const std::string& variable, GLint valor);

		void setUniform(const std::string& nombreSP, const std::string& variable, float valor);

		void activarSubrutina(const std::string& nombreSP, GLenum tipoShader, const std::string& nombreSubrutina);

		void activarMultiplesSubrutinas(const std::string& nombreSP, GLenum tipoShader,
										const std::vector<std::string>& nombreUniform,
										const std::vector<std::string>& nombreSubrutina);

		ComputeShader* getComputeShader(const std::string& name);
	};
}


#endif //PAG_SHADERMANAGER_H
