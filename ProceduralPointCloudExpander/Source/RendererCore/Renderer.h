//
// Created by Niskp on 15/09/2021.
//

#ifndef RENDERER_H
#define RENDERER_H

#include "Camara.h"
#include "PointCloudHQRenderer.h"
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

		vec3 colorFondo = { 0.1, 0.1, 0.15 };
		float pointSize = 1.0f;
		unsigned currentScreenshot = 0;
		std::vector<std::pair<std::string, std::string>> pendingScreenshots;

		ProceduralGenerator procGenerator;
		Camara camara;
		std::unique_ptr<PointCloudHQRenderer> hqRenderer;

		bool hqrCompiled = true;
	public:

		bool hqr;
		float distanceThreshold = 1.0000001f;

		virtual ~Renderer();

		static Renderer* getInstancia();

		void inicializaOpenGL() const;

		void refrescar();

		void actualizarColorFondo() const;

		void cargaModelo(const std::string& path, const bool& newScene, const unsigned& pointsPerVoxel);

		void screenshot(const std::string& filename);

		void pendingScreenshot(const std::string& filename, const std::string& modelKey) const;

		void addPendingScreenshot(const std::string& filename, const std::string& modelKey);

		void setColorFondo(vec3 color);

		static const GLubyte* getPropiedadGL(GLenum propiedad);

		void setViewport(GLint x, GLint y, GLsizei width, GLsizei height);

		Camara& getCamara();

		vec3& getColorFondo();

		float getPointSize() const;

		bool hqrCompatible() const;

		void setPointSize(float pointS);

		void setCameraFocus(const AABB& aabb);

		ProceduralGenerator* getProceduralGenerator();
	};
}


#endif //RENDERER_H
