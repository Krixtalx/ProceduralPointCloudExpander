//
// Created by Niskp on 15/09/2021.
//

#ifndef RENDERER_H
#define RENDERER_H

#include "PointCloud.h"
#include "Camara.h"
#include "ProceduralUtils/ProceduralGenerator.h"

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
		static Renderer* instancia; ///< Puntero al único objeto de la clase
		Renderer();

		glm::vec3 colorFondo = { 0.1, 0.1, 0.15 };
		float pointSize = 1.0f;

		ProceduralGenerator procGenerator;

		PPCX::Camara camara;

	public:
		virtual ~Renderer();

		static Renderer* getInstancia();

		void inicializaOpenGL() const;

		void refrescar();

		void actualizarColorFondo() const;

		void cargaModelo(const std::string& path, const bool& newScene);

		void setColorFondo(glm::vec3 color);

		const GLubyte* getPropiedadGL(GLenum propiedad);

		void setViewport(GLint x, GLint y, GLsizei width, GLsizei height);

		Camara& getCamara();

		glm::vec3& getColorFondo();

		float getPointSize() const;

		void setPointSize(float pointS);

		ProceduralGenerator* getProceduralGenerator();
	};
}


#endif //RENDERER_H
