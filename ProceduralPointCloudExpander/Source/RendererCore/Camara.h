//
// Created by Niskp on 18/10/2021.
//

#ifndef PPCX_CAMARA_H
#define PPCX_CAMARA_H

#include "stdafx.h"

namespace PPCX {
	class Camara {
	private:
		//Parámetros intrínsecos de la camara
		glm::vec3 posicion;
		glm::vec3 puntoMira;
		glm::vec3 up;

		//Ejes locales
		glm::vec3 u;
		glm::vec3 v;
		glm::vec3 n;

		//Planos de clipping
		GLfloat zNear;
		GLfloat zFar;

		//Campo de vision de la cámara
		GLfloat fovX;
		GLfloat fovY;

		//Dimensiones del viewport
		GLuint alto;
		GLuint ancho;

		float speedMultiplier = 1.0f;

		//Orthogonal camara parameters
		glm::vec2 minPoint;
		glm::vec2 maxPoint;
		bool perspective = true;

		Camara* backup;

		void calcularEjes();

		void calcularFovY();

	public:
		Camara();

		Camara(const glm::vec3 &posicion, const glm::vec3 &puntoMira, const glm::vec3 &up, GLfloat zNear, GLfloat zFar,
		       GLfloat fovX, GLuint alto, GLuint ancho);

		void truck(float mov);

		void dolly(float mov);

		void boom(float mov);

		void crane(float mov);

		void zoom(float angulo);

		void pan(float mov);

		void tilt(float mov);

		void orbitX(float mov);

		void orbitY(float mov);

		void increaseZFar(float mov);

		void reset();

		glm::mat4 matrizMVP() const;

		glm::mat4 matrizMV() const;

		GLfloat aspecto() const;

		void setAlto(GLuint alto);

		void setAncho(GLuint ancho);

		void setPosicion(glm::vec3 pos);

		void setPuntoMira(glm::vec3 punto);

		void setSpeedMultiplier(float speed);
		
		void setOrthoPoints(glm::vec2 minPoint, glm::vec2 maxPoint);

		void updateOrtho();

		GLuint getAncho() const;

		GLuint getAlto() const;

		void changeCamaraType();
	};
}


#endif //PPCX_CAMARA_H
