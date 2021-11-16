//
// Created by Niskp on 08/10/2021.
//

#ifndef PAG_MODELO_H
#define PAG_MODELO_H


#include "stdafx.h"
#include "RenderOptions.h"
#include "Utilities/Point.h"

namespace PAG {
	class pointCloud {
	private:
		GLuint idVAO{};
		GLuint idVBO;
		GLuint idIBO;
		std::vector<PointModel> vbo;
		std::vector<GLuint> ibo;
		std::string shaderProgram;
		glm::vec3 posicion;

	public:
		pointCloud(std::string shaderProgram, glm::vec3 pos = {0, 0, 0});

		pointCloud(pointCloud &orig);

		~pointCloud();

		void nuevoVBO(std::vector<PointModel> datos, GLenum freqAct);

		void nuevoIBO(std::vector<GLuint> datos, GLenum freqAct);

		void dibujarModelo(glm::mat4 matrizMVP);
	};
}


#endif //PAG_MODELO_H
