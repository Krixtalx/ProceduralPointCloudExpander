//
// Created by Niskp on 14/11/2021.
//

#ifndef PAG_LUZ_H
#define PAG_LUZ_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include "RenderOptions.h"

namespace PPCX {
	class Luz {
	private:
		glm::vec3 ia{};
		glm::vec3 id;
		glm::vec3 is;

		glm::vec3 posicion{};
		glm::vec3 direccion{};
		float gamma{};
		GLuint exponenteBordes{};

		PPCX::tipoLuz tipoLuz;
		GLuint idMapaSombras = UINT_MAX;

		void inicializarMapaSombras();

	public:
		Luz(const glm::vec3 &ia);

		Luz(const glm::vec3 &id, const glm::vec3 &is, const glm::vec3 &posicionOdireccion, const bool &puntual);

		Luz(const glm::vec3 &id, const glm::vec3 &is, const glm::vec3 &posicion, const glm::vec3 &direccion,
		    float gamma, GLuint exponenteBordes);

		void aplicarLuz(const std::string &shader, const glm::mat4 &matriz) const;

		PPCX::tipoLuz getTipoLuz() const;

		bool emiteSombras() const;

		glm::mat4 generarMatrizVPMS() const;

		GLuint getIdMapaSombras() const;
	};
}

#endif //PAG_LUZ_H
