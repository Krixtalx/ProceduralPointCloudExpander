//
// Created by Niskp on 31/10/2021.
//

#ifndef PAG_MATERIAL_H
#define PAG_MATERIAL_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include "RenderOptions.h"
#include <vector>

namespace PPCX {
	class Material {
	private:
		glm::vec3 ambiente;
		glm::vec3 difuso;
		glm::vec3 especular;
		GLuint phong;
		std::vector<GLuint> idTextura;

		GLuint cargarTextura(const std::string &path);

	public:
		Material(const glm::vec3 &ambiente, const glm::vec3 &difuso, const glm::vec3 &especular, GLuint phong);

		Material(const glm::vec3 &ambiente, const glm::vec3 &difuso, const glm::vec3 &especular, GLuint phong,
		         const std::string &rutaTextura);

		Material(const glm::vec3 &ambiente, const glm::vec3 &difuso, const glm::vec3 &especular, GLuint phong,
		         const std::string &rutaTextura, const std::string &rutaNormalMap);

		~Material();

		const glm::vec3 &getAmbiente() const;

		const glm::vec3 &getDifuso() const;

		const glm::vec3 &getEspecular() const;

		GLuint getPhong() const;

		GLuint getIdTextura(tipoTextura tipo) const;
	};
}


#endif //PAG_MATERIAL_H
