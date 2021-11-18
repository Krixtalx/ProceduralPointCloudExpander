//
// Created by Niskp on 08/10/2021.
//

#ifndef PAG_MODELO_H
#define PAG_MODELO_H

#include <GeometryUtils/AABB.h>

#include "Utilities/Point.h"

namespace PPCX {
	class PointCloud {
	private:
		GLuint idVAO{};
		GLuint idVBO;
		GLuint idIBO;
		std::vector<PointModel> vbo;
		std::vector<GLuint> ibo;
		std::string shaderProgram;
		glm::vec3 posicion;
		AABB aabb;

		void nuevoVBO(std::vector<PointModel> datos, GLenum freqAct);

		void nuevoIBO(std::vector<GLuint> datos, GLenum freqAct);

	public:
		PointCloud(std::string shaderProgram, glm::vec3 pos = {0, 0, 0});

		PointCloud(PointCloud &orig);

		~PointCloud();

		void nuevoPunto(const PointModel& punto);

		void nuevosPuntos(const std::vector<PointModel>& puntos);
		
		void dibujarModelo(glm::mat4 matrizMVP);
	};
}


#endif //PAG_MODELO_H
