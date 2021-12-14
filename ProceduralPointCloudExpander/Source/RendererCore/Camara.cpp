//
// Created by Niskp on 18/10/2021.
//

#include "stdafx.h"
#include "Camara.h"
#include "RenderOptions.h"


/**
 * Constructor por defecto. Inicializa la cámara con unos parametros predeterminados
 */
PPCX::Camara::Camara() : posicion(0, 3.0f, 10.0f), puntoMira(0, 0, 0),
up(0, 0, 1), zNear(0.5f), zFar(200), alto(altoVentanaPorDefecto),
ancho(anchoVentanaPorDefecto) {

	fovX = glm::radians(60.0);
	calcularFovY();
	calcularEjes();
}

/**
 * Constructor parametrizado. Inicializa la cámara con los parametros que se pasan como parametro
 * @param posicion en la que se establecerá la camara
 * @param puntoMira punto al que mira la camara
 * @param up vector que indica la dirección "arriba" de la camara
 * @param zNear plano de clipping cercano
 * @param zFar plano de clipping lejano
 * @param fovX campo de visión en el eje X
 * @param alto del viewport
 * @param ancho del viewport
 */
PPCX::Camara::Camara(const glm::vec3& posicion, const glm::vec3& puntoMira, const glm::vec3& up, GLfloat zNear,
					 GLfloat zFar, GLfloat fovX, GLuint alto, GLuint ancho) : posicion(posicion), puntoMira(puntoMira),
	up(up), zNear(zNear), zFar(zFar),
	alto(alto), ancho(ancho) {
	this->fovX = glm::radians(fovX);
	calcularFovY();
	calcularEjes();
}

/**
 * Calcula la matriz de modelado visión y proyección y la devuelve.
 * @return matrizMVP
 */
glm::mat4 PPCX::Camara::matrizMVP() const {
	//std::cout << puntoMira.x << " - " << puntoMira.y << " - " << puntoMira.z << std::endl;
	const glm::mat4 vision = lookAt(posicion, puntoMira, up);
	const glm::mat4 proyeccion = glm::perspective(fovY, aspecto(), zNear, zFar);
	//Multiplicamos de manera inversa: Modelado-Vision-Proyeccion -> Proyeccion-Vision-Modelado
	return proyeccion * vision; //Devuelve solo proyección*vision. El modelado lo aplicará el modelo
}

/**
 * Calcula la matriz de modelado y visión y la devuelve.
 * @return matrizMV
 */
glm::mat4 PPCX::Camara::matrizMV() const {
	return lookAt(posicion, puntoMira, up); //Devuelve solo vision. El modelado lo aplicará el modelo
}

/**
 * Actualiza los ejes locales de la cámara recalculando su valor.
 */
void PPCX::Camara::calcularEjes() {
	n = normalize(posicion - puntoMira);

	if (all(equal(n, up, 0.001f))) {
		u = normalize(cross(glm::vec3(0.0f, 0.0f, 1.0f), n));
	} else if (all(equal(n, -up, 0.001f))) {
		u = normalize(cross(glm::vec3(0.0f, 0.0f, -1.0f), n));
	} else {
		u = normalize(cross(up, n));
	}
	v = normalize(cross(n, u));
}

/**
 * Calcula el fovY equivalente al fovX actual
 */
void PPCX::Camara::calcularFovY() {
	this->fovY = 2 * glm::atan(tan(this->fovX / 2) / aspecto());
}


/**
 * ------------------------------Movimientos de la camara------------------------
 */

 /**
  * Movimiento truck de la camara. Mueve la camara hacia delante o hacia atrás
  * @param mov magnitud del movimiento
  */
void PPCX::Camara::truck(float mov) {
	const glm::mat4 translacion = translate(n * mov * speedMultiplier);
	posicion = glm::vec3(translacion * glm::vec4(posicion, 1));
	puntoMira = glm::vec3(translacion * glm::vec4(puntoMira, 1));
}

/**
 * Movimiento dolly de la camara. Mueve la camara hacia la izquierda o hacia la derecha
 * @param mov magnitud del movimiento
 */
void PPCX::Camara::dolly(float mov) {
	const glm::mat4 translacion = translate(u * mov * speedMultiplier);
	posicion = glm::vec3(translacion * glm::vec4(posicion, 1));
	puntoMira = glm::vec3(translacion * glm::vec4(puntoMira, 1));
}

/**
 * Movimiento boom de la cámara. Mueve la cámara hacia arriba
 * @param mov magnitud del movimiento
 */
void PPCX::Camara::boom(float mov) {
	const glm::mat4 translacion = translate(v * mov * speedMultiplier);
	posicion = glm::vec3(translacion * glm::vec4(posicion, 1));
	puntoMira = glm::vec3(translacion * glm::vec4(puntoMira, 1));
}

/**
 * Movimiento boom de la cámara. Mueve la cámara hacia abajo
 * @param mov magnitud del movimiento
 */
void PPCX::Camara::crane(float mov) {
	boom(-mov);
}

/**
 * Movimiento pan de la cámara. Rota la cámara horizontalmente
 * @param mov magnitud del movimiento
 */
void PPCX::Camara::pan(float mov) {
	const glm::mat4 rotacion = rotate(glm::radians(mov), v);
	puntoMira = glm::vec3(rotacion * glm::vec4(puntoMira - posicion, 1)) + posicion;
	u = glm::vec3(rotacion * glm::vec4(u, 0.0f));
	v = glm::vec3(rotacion * glm::vec4(v, 0.0f));
	n = glm::vec3(rotacion * glm::vec4(n, 0.0f));
	up = normalize(cross(n, u));
}

/**
 * Movimiento tilt de la cámara. Rota la cámara verticalmente
 * @param mov magnitud del movimiento
 */
void PPCX::Camara::tilt(float mov) {
	const glm::mat4 rotacion = rotate(glm::radians(mov), u);

	const glm::vec3 aux = glm::vec3(rotacion * glm::vec4(n, 0.0f));
	float alpha = glm::acos(dot(aux, glm::vec3(0.0f, 1.0f, 0.0f)));

	if (alpha < mov || alpha >(glm::pi<float>() - mov)) {
		return;
	}

	puntoMira = glm::vec3(rotacion * glm::vec4(puntoMira - posicion, 1)) + posicion;
	u = glm::vec3(rotacion * glm::vec4(u, 0.0f));
	v = glm::vec3(rotacion * glm::vec4(v, 0.0f));
	n = aux;
	up = normalize(cross(n, u));
}

/**
 * Orbita en longitud alrededor del punto al que se mira.
 * @param mov magnitud del movimiento
 */
void PPCX::Camara::orbitX(float mov) {
	const glm::mat4 rotacion = rotate(glm::radians(mov), glm::vec3(.0f, .0f, 1.0f));
	posicion = glm::vec3(rotacion * glm::vec4(posicion - puntoMira, 1)) + puntoMira;
	u = glm::vec3(rotacion * glm::vec4(u, 0.0f));
	v = glm::vec3(rotacion * glm::vec4(v, 0.0f));
	n = glm::vec3(rotacion * glm::vec4(n, 0.0f));
	up = normalize(cross(n, u));
}

/**
 * Orbita en latitud alrededor del punto al que se mira.
 * @param mov magnitud del movimiento
 */
void PPCX::Camara::orbitY(float mov) {
	const glm::mat4 rotacion = rotate(glm::radians(mov), u);
	posicion = glm::vec3(rotacion * glm::vec4(posicion - puntoMira, 1)) + puntoMira;
	u = glm::vec3(rotacion * glm::vec4(u, 0.0f));
	v = glm::vec3(rotacion * glm::vec4(v, 0.0f));
	n = glm::vec3(rotacion * glm::vec4(n, 0.0f));
	up = normalize(cross(n, u));
}

void PPCX::Camara::increaseZFar(float mov) {
	zFar += mov;
}

/**
 * Realiza el zoom modificando el fov
 * @param angulo variación del ángulo
 */
void PPCX::Camara::zoom(float angulo) {
	fovX += glm::radians(angulo);
	if (fovX < 0)
		fovX = 0;
	if (fovX > glm::pi<float>())
		fovX = glm::pi<float>();

	calcularFovY();
}

/**
 * Situa la cámara en la posición por defecto.
 */
void PPCX::Camara::reset() {
	this->posicion = backup->posicion;
	this->puntoMira = backup->puntoMira;
	this->alto = backup->alto;
	this->ancho = backup->ancho;
	this->up = glm::vec3(.0f, .0f, 1.0f);
	calcularEjes();
}


/**
 * ------------------------------Getters y setters------------------------
 */

 /**
  * Calcula y devuelve la relacion de aspecto
  * @return relacion de aspecto del viewport
  */
GLfloat PPCX::Camara::aspecto() const {
	return (GLfloat(ancho) / GLfloat(alto));
}

void PPCX::Camara::setAlto(GLuint alto) {
	Camara::alto = alto;
	delete backup;
	backup = new Camara(*this);
}

void PPCX::Camara::setAncho(GLuint ancho) {
	Camara::ancho = ancho;
	delete backup;
	backup = new Camara(*this);
}

void PPCX::Camara::setPosicion(glm::vec3 pos) {
	posicion = pos;
	calcularEjes();
	delete backup;
	backup = new Camara(*this);
}

void PPCX::Camara::setPuntoMira(glm::vec3 punto) {
	puntoMira = punto;
	calcularEjes();
	delete backup;
	backup = new Camara(*this);
}

void PPCX::Camara::setSpeedMultiplier(float speed) {
	speedMultiplier = speed;
}
