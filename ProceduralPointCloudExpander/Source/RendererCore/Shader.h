//
// Created by Niskp on 01/10/2021.
//

#ifndef PAG_SHADER_H
#define PAG_SHADER_H

namespace PPCX {
	class Shader {
		friend class ShaderManager;
		
		std::string nombreShader;
		GLuint idShader{};
		GLenum tipoShader{};

		void cargaShader(const std::string &ruta) const;

		void compilaShader() const;

		void compruebaErroresShader() const;

	protected:
		Shader(std::string nombreShader, GLenum tipoShader, const std::string &ruta);
		
	public:

		Shader(const Shader &orig)=delete;

		virtual ~Shader();

		GLuint getShaderId() const;

		GLenum getTipoShader() const;
	};
}


#endif //PAG_SHADER_H
