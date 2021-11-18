//
// Created by Niskp on 01/10/2021.
//

#ifndef PAG_SHADER_H
#define PAG_SHADER_H

namespace PAG {
	class Shader {
		friend class ShaderManager;

	private:
		std::string nombreShader;
		GLuint idShader{};
		GLenum tipoShader{};

		void cargaShader(const std::string &ruta) const;

		void compilaShader();

		void compruebaErroresShader() const;

	protected:
		Shader(std::string nombreShader, GLenum tipoShader, const std::string &ruta);
		
	public:

		Shader(const Shader &orig)=delete;

		~Shader();

		GLuint getShaderId() const;

		GLenum getTipoShader() const;
	};
}


#endif //PAG_SHADER_H
