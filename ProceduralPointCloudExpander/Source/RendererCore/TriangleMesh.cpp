//
// Created by Niskp on 08/10/2021.
//

#include "stdafx.h"
#include "TriangleMesh.h"
#include "ShaderManager.h"
#include "MaterialManager.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <utility>
#include <stdexcept>
#include <climits>

/**
 * Constructor parametrizado
 * @param shaderProgram que se usará para renderizar el modelo
 * @param path ruta de la que se cargará el modelo
 * @param pos
 * @param rot
 * @param scale
 */
PPCX::TriangleMesh::TriangleMesh(std::string shaderProgram, const std::string& path, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale) :
	Model(std::move(shaderProgram), pos, rot, scale), modo(mallaTriangulos) {

	//Ponemos tanto espacios en el vector de VBO como parametros tengamos para el shader.
	idVBO.resize(numParamShader());
	vbos.resize(numParamShader());
	for (unsigned int& i : idVBO) {
		i = UINT_MAX;
	}
	//Ponemos tanto espacios en el vector de IBO como modos de dibujado tengamos.
	idIBO.resize(numModosDibujo());
	ibos.resize(numModosDibujo());
	for (unsigned int& i : idIBO) {
		i = UINT_MAX;
	}

	if (path != "NULL")
		cargaModelo(path);
}

/**
 * Constructor copia. Copia el numVertices y el shaderProgram y realiza una nueva instanciacion de los vbos e ibos
 * @param orig
 */
PPCX::TriangleMesh::TriangleMesh(TriangleMesh& orig) : Model(orig),
                                                       vbos(orig.vbos), ibos(orig.ibos), material(orig.material) {

	//Ponemos tanto espacios en el vector de VBO como parametros tengamos para el shader.
	idVBO.resize(numParamShader());
	//Ponemos tanto espacios en el vector de IBO como modos de dibujado tengamos.
	idIBO.resize(numModosDibujo());

	for (int i = 0; i < vbos.size(); ++i) {
		nuevoVBO(static_cast<paramShader>(i), vbos[i], GL_STATIC_DRAW);
	}
	for (int i = 0; i < ibos.size(); ++i) {
		nuevoIBO(static_cast<modoDibujado>(i), ibos[i], GL_STATIC_DRAW);
	}
}

/**
 * Destructor. Libera todos los recursos reservados a OpenGL.
 */
PPCX::TriangleMesh::~TriangleMesh() {
	for (unsigned int& vbo : idVBO) {
		if (vbo != UINT_MAX)
			glDeleteBuffers(1, &vbo);
	}
	for (unsigned int& ibo : idIBO) {
		if (ibo != UINT_MAX)
			glDeleteBuffers(1, &ibo);
	}
	glDeleteVertexArrays(1, &idVAO);
}

/**
 * Instancia un VBO en el contexto OpenGL y lo guarda en vbos
 * @param tipoDato parametro del shader que representa el vbo
 * @param datos a instanciar
 * @param freqAct GLenum que indica con que frecuencia se van a modificar los vertices. GL_STATIC_DRAW siempre por ahora
 */
void PPCX::TriangleMesh::nuevoVBO(paramShader tipoDato, const std::vector<glm::vec3>& datos, const GLenum freqAct) {
	//Si hay un buffer de este tipo instanciado, lo eliminamos
	if (idVBO[tipoDato] != UINT_MAX) {
		glDeleteBuffers(1, &idVBO[tipoDato]);
	}
	vbos[tipoDato] = datos;
	glBindVertexArray(idVAO);
	glGenBuffers(1, &idVBO[tipoDato]);
	glBindBuffer(GL_ARRAY_BUFFER, idVBO[tipoDato]);
	glBufferData(GL_ARRAY_BUFFER, datos.size() * sizeof(glm::vec3), datos.data(), freqAct);
	glVertexAttribPointer(tipoDato, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3),
						  nullptr);
	glEnableVertexAttribArray(tipoDato);
}

/**
 * Instancia un VBO en el contexto OpenGL y lo guarda en textura
 * @param tipoDato parametro del shader que representa el vbo
 * @param datos a instanciar
 * @param freqAct GLenum que indica con que frecuencia se van a modificar los vertices. GL_STATIC_DRAW siempre por ahora
 */
void PPCX::TriangleMesh::nuevoVBO(const paramShader tipoDato, const std::vector<glm::vec2>& datos, const GLenum freqAct) {
	//Si hay un buffer de este tipo instanciado, lo eliminamos
	if (idVBO[tipoDato] != UINT_MAX) {
		glDeleteBuffers(1, &idVBO[tipoDato]);
	}
	textura = datos;
	glBindVertexArray(idVAO);
	glGenBuffers(1, &idVBO[tipoDato]);
	glBindBuffer(GL_ARRAY_BUFFER, idVBO[tipoDato]);
	glBufferData(GL_ARRAY_BUFFER, datos.size() * sizeof(glm::vec2), datos.data(), freqAct);
	glVertexAttribPointer(tipoDato, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2),
						  nullptr);
	glEnableVertexAttribArray(tipoDato);
}

/**
 * Instancia un IBO en el contexto OpenGL y lo guarda en ibos
 * @param modo modo de dibujado que representa el ibo
 * @param datos a instanciar
 * @param freqAct GLenum que indica con que frecuencia se van a modificar los vertices. GL_STATIC_DRAW siempre por ahora
 */
void PPCX::TriangleMesh::nuevoIBO(modoDibujado modo, const std::vector<GLuint>& datos, GLenum freqAct) {
	//Si hay un buffer de este tipo instanciado, lo eliminamos
	if (idIBO[modo] != UINT_MAX) {
		glDeleteBuffers(1, &idIBO[modo]);
	}
	ibos[modo] = datos;
	glBindVertexArray(idVAO);
	glGenBuffers(1, &idIBO[modo]);
	glBindBuffer(GL_ARRAY_BUFFER, idIBO[modo]);
	glBufferData(GL_ARRAY_BUFFER, datos.size() * sizeof(GLuint), datos.data(), freqAct);
}


void PPCX::TriangleMesh::drawModel(const glm::mat4& MVPMatrix) {
	if (visible) {
		try {
			auto matrixMVP = MVPMatrix * mModelado;
			ShaderManager::getInstance()->activarSP(shaderProgram);
			ShaderManager::getInstance()->setUniform(this->shaderProgram, "matrizMVP", matrixMVP);
			ShaderManager::getInstance()->setUniform(this->shaderProgram, "matrizMV", glm::mat4(1.0f));
			ShaderManager::getInstance()->setUniform(this->shaderProgram, "matrizMS", glm::mat4(1.0f));

			const std::vector<std::string> nombreUniforms = { "luzElegida", "colorElegido", "normalMap" };

			if (modo == wireframe) {
				const std::vector<std::string> nombreSubrutinas = { "colorDefecto", "colorMaterial", "noUsarNormalMap" };
				ShaderManager::getInstance()->activarMultiplesSubrutinas(this->shaderProgram, GL_FRAGMENT_SHADER,
				                                                          nombreUniforms, nombreSubrutinas);
			} else {

				std::vector<std::string> nombreSubrutinas;

				const GLuint idTextura = MaterialManager::getInstancia()->getMaterial(material)->getIdTextura(
					texturaColor);
				if (usarTexturas && idTextura != UINT_MAX) {
					nombreSubrutinas.emplace_back("colorTextura");
					nombreSubrutinas.emplace_back("noUsarNormalMap");
					ShaderManager::getInstance()->setUniform(shaderProgram, "muestreador", (GLint)0);
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, idTextura);
				} else {
					nombreSubrutinas.emplace_back("colorMaterial");
					nombreSubrutinas.emplace_back("noUsarNormalMap");
				}

				glBindVertexArray(idVAO);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idIBO[modo]);
				glPolygonMode(GL_FRONT_AND_BACK, getGLDrawMode(modo));
				glDrawElements(GL_TRIANGLES, ibos[modo].size(), GL_UNSIGNED_INT, nullptr);
			}

		} catch (std::runtime_error& e) {
			throw e;
		}
	}

}

/**
 * Función a la que se llama cuando se debe de dibujar el modelo
 * @param matrizMVP
 * @param tipoLuz Tipo de luz que se esta renderizando actualmente. Es necesario para la activación de las subrutinas del shader
 */
void PPCX::TriangleMesh::dibujarModelo(glm::mat4 matrizMVP, glm::mat4 matrizMV, glm::mat4 matrizMS, tipoLuz tipoLuz) {
	if (visible)
		try {
		matrizMVP = matrizMVP * mModelado;
		matrizMV = matrizMV * mModelado;
		matrizMS = matrizMS * mModelado;

		ShaderManager::getInstance()->activarSP(shaderProgram);
		ShaderManager::getInstance()->setUniform(this->shaderProgram, "matrizMVP", matrizMVP);
		ShaderManager::getInstance()->setUniform(this->shaderProgram, "matrizMV", matrizMV);
		ShaderManager::getInstance()->setUniform(this->shaderProgram, "matrizMS", matrizMS);
		const std::vector<std::string> nombreUniforms = { "luzElegida", "colorElegido", "normalMap" };

		if (modo == wireframe) {
			const std::vector<std::string> nombreSubrutinas = { "colorDefecto", "colorMaterial", "noUsarNormalMap" };
			ShaderManager::getInstance()->activarMultiplesSubrutinas(this->shaderProgram, GL_FRAGMENT_SHADER,
			                                                          nombreUniforms, nombreSubrutinas);
		} else {
			const glm::vec3 ambiente = MaterialManager::getInstancia()->getMaterial(
				this->material)->getAmbiente();
			const glm::vec3 difusa = MaterialManager::getInstancia()->getMaterial(
				this->material)->getDifuso();
			const glm::vec3 especular = MaterialManager::getInstancia()->getMaterial(
				this->material)->getEspecular();
			const GLuint phong = MaterialManager::getInstancia()->getMaterial(
				this->material)->getPhong();
			ShaderManager::getInstance()->setUniform(this->shaderProgram, "Ka", ambiente);
			ShaderManager::getInstance()->setUniform(this->shaderProgram, "Kd", difusa);
			ShaderManager::getInstance()->setUniform(this->shaderProgram, "Ks", especular);
			ShaderManager::getInstance()->setUniform(this->shaderProgram, "phong", phong);

			std::vector<std::string> nombreSubrutinas;
			if (tipoLuz == tipoLuz::ambiente) {
				nombreSubrutinas.emplace_back("luzAmbiente");
			} else if (tipoLuz == tipoLuz::puntual) {
				nombreSubrutinas.emplace_back("luzPuntual");
			} else if (tipoLuz == tipoLuz::direccional) {
				nombreSubrutinas.emplace_back("luzDireccional");
			} else {
				nombreSubrutinas.emplace_back("luzFoco");
			}

			const GLuint idTextura = MaterialManager::getInstancia()->getMaterial(material)->getIdTextura(
				texturaColor);
			const GLuint idNormalMap = MaterialManager::getInstancia()->getMaterial(material)->getIdTextura(
				normalMap);
			if (usarTexturas && usarNormalMap && idTextura != UINT_MAX && idNormalMap != UINT_MAX) {
				nombreSubrutinas.emplace_back("colorTextura");
				nombreSubrutinas.emplace_back("siUsarNormalMap");
				ShaderManager::getInstance()->setUniform(shaderProgram, "muestreador", (GLint)0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, idTextura);

				ShaderManager::getInstance()->setUniform(shaderProgram, "muestreadorNormalMap", (GLint)1);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, idNormalMap);
			} else if (usarTexturas && idTextura != UINT_MAX) {
				nombreSubrutinas.emplace_back("colorTextura");
				nombreSubrutinas.emplace_back("noUsarNormalMap");
				ShaderManager::getInstance()->setUniform(shaderProgram, "muestreador", (GLint)0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, idTextura);
			} else {
				nombreSubrutinas.emplace_back("colorMaterial");
				nombreSubrutinas.emplace_back("noUsarNormalMap");
			}

			ShaderManager::getInstance()->activarMultiplesSubrutinas(this->shaderProgram, GL_FRAGMENT_SHADER,
			                                                          nombreUniforms, nombreSubrutinas);
		}
		glBindVertexArray(idVAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idIBO[modo]);
		glPolygonMode(GL_FRONT_AND_BACK, getGLDrawMode(modo));
		glDrawElements(GL_TRIANGLES, ibos[modo].size(), GL_UNSIGNED_INT, nullptr);


	} catch (std::runtime_error& e) {
		throw e;
	}
}

/**
 * Dibuja el modelo tal y como se requiere para el calculo del mapa de sombras
 */
void PPCX::TriangleMesh::dibujarModeloParaSombras() {
	if (visible) {
		glBindVertexArray(idVAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idIBO[modo]);
		glPolygonMode(GL_FRONT_AND_BACK, getGLDrawMode(modo));
		glDrawElements(GL_TRIANGLES, ibos[modo].size(), GL_UNSIGNED_INT, nullptr);
	}
}

/**
 * Traduce de modoDibujado a los modos de dibujado de OpenGL
 * @param modo de modoDibujado
 * @return GLenum que indica como debe de dibujarse
 */
GLenum PPCX::TriangleMesh::getGLDrawMode(modoDibujado modo) {
	switch (modo) {
		case nubePuntos:
			return GL_POINT;
		case wireframe:
			return GL_LINE;
		default:
			return GL_FILL;
	}
}

void PPCX::TriangleMesh::setMaterial(const std::string& material) {
	TriangleMesh::material = material;
}


const std::string& PPCX::TriangleMesh::getShaderProgram() const {
	return shaderProgram;
}

/**
 * Método para cargar un modelo 3D
 * @param path ruta en la que se encuentra el modelo
 */
void PPCX::TriangleMesh::cargaModelo(const std::string& path) {
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(path, aiProcess_JoinIdenticalVertices | aiProcess_Triangulate |
										   aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		throw std::runtime_error("[TriangleMesh::cargaModelo]: Error en la carga del modelo con Assimp");
	}

	procesarNodo(scene->mRootNode, scene);
}

/**
 * Método que procesa los distintos nodos del modelo que carga ASIMP
 * @param node nodo de ASIMP
 * @param scene escena de ASIMP
 */
void PPCX::TriangleMesh::procesarNodo(aiNode* node, const aiScene* scene) {
	// process all the node's meshes (if any)
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		procesarMalla(mesh, scene);
	}
	// then do the same for each of its children
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		procesarNodo(node->mChildren[i], scene);
	}
}

/**
 * Método que procesa la malla cargada
 * @param mesh malla de ASIMP
 * @param scene escena de ASIMP
 */
void PPCX::TriangleMesh::procesarMalla(const aiMesh* mesh, const aiScene* scene) {
	glm::vec3 vec;
	glm::vec2 vecTex;
	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		//Vertices
		vec.x = mesh->mVertices[i].x;
		vec.y = mesh->mVertices[i].y;
		vec.z = mesh->mVertices[i].z;
		vbos[posicion].push_back(vec);

		//Normales
		vec.x = mesh->mNormals[i].x;
		vec.y = mesh->mNormals[i].y;
		vec.z = mesh->mNormals[i].z;
		vbos[normal].push_back(vec);

		if (mesh->mTextureCoords[0]) {
			//Coordenadas de textura
			vecTex.x = mesh->mTextureCoords[0][i].x;
			vecTex.y = mesh->mTextureCoords[0][i].y;
		} else {
			vecTex = { 0, 0 };
		}
		textura.push_back(vecTex);

		if (mesh->mTangents) // Si hay tangentes, también habrá bitangentes
		{
			//Tangentes
			glm::vec3 t;
			t.x = mesh->mTangents[i].x;
			t.y = mesh->mTangents[i].y;
			t.z = mesh->mTangents[i].z;
			vbos[tangente].push_back(t);

			//Bitangentes
			glm::vec3 bt;
			bt.x = mesh->mBitangents[i].x;
			bt.y = mesh->mBitangents[i].y;
			bt.z = mesh->mBitangents[i].z;
			vbos[bitangente].push_back(bt);
		}

	}
	//Creamos los VBOs
	this->nuevoVBO(posicion, vbos[posicion], GL_STATIC_DRAW);
	this->nuevoVBO(normal, vbos[normal], GL_STATIC_DRAW);
	this->nuevoVBO(tangente, vbos[tangente], GL_STATIC_DRAW);
	this->nuevoVBO(bitangente, vbos[bitangente], GL_STATIC_DRAW);
	this->nuevoVBO(PPCX::textura, textura, GL_STATIC_DRAW);

	//Cargamos las caras del modelo
	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		const aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			ibos[mallaTriangulos].push_back(face.mIndices[j]);
	}

	//Creamos los IBOs
	this->nuevoIBO(mallaTriangulos, ibos[mallaTriangulos], GL_STATIC_DRAW);
	this->nuevoIBO(wireframe, ibos[mallaTriangulos], GL_STATIC_DRAW);
	//for (int i = 0; i < vbos[posicion].size(); i++) {
	//	ibos[nubePuntos][i] = i;
	//}
	//this->nuevoIBO(PPCX::nubePuntos, ibos[nubePuntos], GL_STATIC_DRAW);
}

void PPCX::TriangleMesh::cambiarUsoTextura() {
	usarTexturas = !usarTexturas;
}

void PPCX::TriangleMesh::cambiarModoDibujado() {
	if (modo == mallaTriangulos)
		modo = wireframe;
	else
		modo = mallaTriangulos;
}

PPCX::modoDibujado PPCX::TriangleMesh::getModo() const {
	return modo;
}

void PPCX::TriangleMesh::cambiarUsoNormalMap() {
	usarNormalMap = !usarNormalMap;
}

const glm::mat4& PPCX::TriangleMesh::getMModelado() const {
	return mModelado;
}