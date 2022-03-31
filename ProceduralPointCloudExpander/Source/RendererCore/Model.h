//
// Created by Niskp on 08/10/2021.
//

#ifndef PAG_MODEL_H
#define PAG_MODEL_H

#include "stdafx.h"

namespace PPCX {
	class Model {
	protected:
		GLuint idVAO{};
		GLuint idVBO;
		GLuint idIBO;

		std::string shaderProgram;
		glm::mat4 mModelado{};

		bool visible = true;

	public:
		Model(std::string shaderProgram, const glm::vec3& pos = { 0, 0, 0 }, const glm::vec3& rot = { 0, 0, 0 }, const glm::vec3& scale = { 1, 1, 1 });

		Model(Model& orig);

		virtual ~Model();

		virtual void drawModel(const glm::mat4& MVPMatrix) = 0;

		bool& getVisibility();
	};
}


#endif //PAG_MODEL_H
