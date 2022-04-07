//
// Created by Niskp on 31/10/2021.
//

#include "stdafx.h"
#include "Material.h"
#include "lodepng.h"
#include "RenderOptions.h"

PPCX::Material::Material(const glm::vec3 &ambiente, const glm::vec3 &difuso, const glm::vec3 &especular, GLuint phong)
		: ambiente(ambiente), difuso(difuso), especular(especular), phong(phong) {
	for (unsigned i = 0; i < numTiposTextura(); ++i) {
		idTextura.push_back(UINT_MAX);
	}
}

/**
 * Constructor parametrizado de Material.
 * @param ambiente valor ambiente del material
 * @param difuso valor difuso del material
 * @param especular valor especular del material
 * @param phong exponente especular del material
 * @param rutaTextura ruta en la que se encuentra la textura a cargar
 */
PPCX::Material::Material(const glm::vec3 &ambiente, const glm::vec3 &difuso, const glm::vec3 &especular, GLuint phong,
                        const std::string &rutaTextura) : ambiente(ambiente),
                                                          difuso(difuso),
                                                          especular(especular),
                                                          phong(phong) {
	for (unsigned i = 0; i < numTiposTextura(); ++i) {
		idTextura.push_back(UINT_MAX);
	}

	idTextura[texturaColor] = cargarTextura(rutaTextura);
}


/**
 * Constructor parametrizado de Material.
 * @param ambiente valor ambiente del material
 * @param difuso valor difuso del material
 * @param especular valor especular del material
 * @param phong exponente especular del material
 * @param rutaTextura ruta en la que se encuentra la imagen que utilizaremos como textura
 * @param rutaNormalMap ruta en la que se encuentra la imagen que utilizaremos como normalMap
 */
PPCX::Material::Material(const glm::vec3 &ambiente, const glm::vec3 &difuso, const glm::vec3 &especular, GLuint phong,
                        const std::string &rutaTextura, const std::string &rutaNormalMap) : ambiente(ambiente),
                                                                                            difuso(difuso),
                                                                                            especular(especular),
                                                                                            phong(phong) {

	for (unsigned i = 0; i < numTiposTextura(); ++i) {
		idTextura.push_back(UINT_MAX);
	}

	idTextura[texturaColor] = cargarTextura(rutaTextura);
	idTextura[normalMap] = cargarTextura(rutaNormalMap);

}

PPCX::Material::~Material() {
	for (unsigned i = 0; i < numTiposTextura(); ++i) {
		glDeleteTextures(1, &idTextura[i]);
	}
}

/**
 * Carga la imagen que se encuentre en el path pasado como parámetro y la incluye como textura en el contexto de OpenGL
 * @param path Ruta de la imagen
 * @return Id asignado por OpenGL a la textura
 */
unsigned int PPCX::Material::cargarTextura(const std::string &path) {
	std::vector<unsigned char> imagen; // Los píxeles de la imagen
	unsigned ancho, alto;
	unsigned error = lodepng::decode(imagen, ancho, alto, path);
	if (error) {
		std::string mensaje = path + " no se pudo cargar";
		throw std::runtime_error(mensaje);
	}
	// La textura se carga del revés, así que vamos a darle la vuelta
	unsigned char *imgPtr = &imagen[0];
	int numeroDeComponentesDeColor = 4;
	int incrementoAncho = ancho * numeroDeComponentesDeColor; // Ancho en bytes
	unsigned char *top = nullptr;
	unsigned char *bot = nullptr;
	unsigned char temp = 0;
	for (int i = 0; i < alto / 2; i++) {
		top = imgPtr + i * incrementoAncho;
		bot = imgPtr + (alto - i - 1) * incrementoAncho;
		for (int j = 0; j < incrementoAncho; j++) {
			temp = *top;
			*top = *bot;
			*bot = temp;
			++top;
			++bot;
		}
	}
	GLuint tmpIdTextura;
	glGenTextures(1, &tmpIdTextura);
	glBindTexture(GL_TEXTURE_2D, tmpIdTextura);
	// Cómo resolver la minificación. En este caso, le decimos que utilice mipmaps, y que aplique interpolación lineal
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	// Cómo resolver la magnificación. En este caso, le decimos que utilice mipmaps, y que aplique interpolación lineal
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	// Cómo pasar de coordenadas de textura a coordenadas en el espacio de la textura en horizontal
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	// Cómo pasar de coordenadas de textura a coordenadas en el espacio de la textura en vertical
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// Finalmente, pasamos la textura a OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ancho, alto, 0, GL_RGBA,
	             GL_UNSIGNED_BYTE, imagen.data());
	glGenerateTextureMipmap(tmpIdTextura);

	return tmpIdTextura;
}

const glm::vec3 &PPCX::Material::getAmbiente() const {
	return ambiente;
}

const glm::vec3 &PPCX::Material::getDifuso() const {
	return difuso;
}

const glm::vec3 &PPCX::Material::getEspecular() const {
	return especular;
}

GLuint PPCX::Material::getPhong() const {
	return phong;
}

/**
 * Devuelve el id de OpenGL del tipo de textura que se requiera
 * @param tipo tipo de textura
 * @return id de OpenGL de la textura. Puede ser UINT_MAX en caso de que no este cargada.
 */
GLuint PPCX::Material::getIdTextura(tipoTextura tipo) const {
	return idTextura[tipo];
}