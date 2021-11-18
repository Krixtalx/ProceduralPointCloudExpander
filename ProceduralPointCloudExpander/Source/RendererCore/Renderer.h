//
// Created by Niskp on 15/09/2021.
//

#ifndef RENDERER_H
#define RENDERER_H

#include "PointCloud.h"
#include "Camara.h"

namespace PPCX {
	/**
	 * @brief Clase encargada de encapsular la gestión del área de dibujo
	 * OpenGL
	 *
	 * Esta clase coordina el renderizado de las escenas OpenGL. Se implementa
	 * aplicando el patrón de diseño Singleton. Está pensada para que las
	 * funciones callback hagan llamadas a sus métodos
	 */

	class Renderer {
	private:
		static Renderer *instancia; ///< Puntero al único objeto de la clase
		Renderer();

		glm::vec3 colorFondo = { 0.2, 0.2, 0.2 };

		bool triangulo = false, tetraedro = false;
		std::vector<PPCX::PointCloud *> modelos;

		PPCX::Camara camara;

	public:
		virtual ~Renderer();

		static Renderer *getInstancia();

		void inicializaOpenGL();

		void refrescar() const;

		void setColorFondo(glm::vec3 color);

		void actualizarColorFondo() const;

		const GLubyte *getPropiedadGL(GLenum propiedad);

		void activarUtilidadGL(GLenum utility);

		void setViewport(GLint x, GLint y, GLsizei width, GLsizei height);

		void limpiarGL(GLbitfield mascara);

		void cargaModelo(const std::string& path);

		Camara &getCamara();

		glm::vec3& getColorFondo();

	};
}


#endif //RENDERER_H
