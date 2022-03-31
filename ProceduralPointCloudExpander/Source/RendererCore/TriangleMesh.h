//
// Created by Niskp on 08/10/2021.
//

#ifndef PPCX_MODELO_H
#define PPCX_MODELO_H


#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <assimp/scene.h>

#include "Model.h"
#include "RenderOptions.h"

namespace PPCX {
	class TriangleMesh : public Model {
	protected:
		std::vector<GLuint> idVBO;
		std::vector<GLuint> idIBO;
		std::vector<std::vector<glm::vec3>> vbos;
		std::vector<std::vector<GLuint>> ibos;
		std::vector<glm::vec2> textura;
		std::string material;
		bool usarNormalMap = true;
		bool usarTexturas = true;

		PPCX::modoDibujado modo;

		GLenum getGLDrawMode(PPCX::modoDibujado modo);

	private:

		void cargaModelo(const std::string& path);

		void procesarNodo(aiNode* node, const aiScene* scene);

		void procesarMalla(const aiMesh* mesh, const aiScene* scene);

	public:

		TriangleMesh(std::string shaderProgram, const std::string& path = "NULL", glm::vec3 pos = { 0, 0, 0 },
					 glm::vec3 rot = { 0, 0, 0 }, glm::vec3 scale = { 1, 1, 1 });

		TriangleMesh(TriangleMesh& orig);

		~TriangleMesh() override;

		void nuevoVBO(PPCX::paramShader tipoDato, const std::vector<glm::vec3>& datos, GLenum freqAct);

		void nuevoVBO(PPCX::paramShader tipoDato, const std::vector<glm::vec2>& datos, GLenum freqAct);

		void nuevoIBO(PPCX::modoDibujado modo, const std::vector<GLuint>& datos, GLenum freqAct);

		void setMaterial(const std::string& material);

		virtual void drawModel(const glm::mat4& MVPMatrix) override;

		virtual void dibujarModelo(glm::mat4 matrizMVP, glm::mat4 matrizMV, glm::mat4 matrizMS, PPCX::tipoLuz tipoLuz);

		virtual void dibujarModeloParaSombras();

		const std::string& getShaderProgram() const;

		void cambiarModoDibujado();

		void cambiarUsoNormalMap();

		void cambiarUsoTextura();

		PPCX::modoDibujado getModo() const;

		const glm::mat4& getMModelado() const;
	};
}


#endif //PPCX_MODELO_H
