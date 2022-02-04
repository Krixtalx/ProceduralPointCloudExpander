//
// Created by Niskp on 08/10/2021.
//

#ifndef PAG_MODEL_H
#define PAG_MODEL_H

#include "Utilities/Point.h"

namespace PPCX {
	class Model {
	protected:
		GLuint idVAO{};
		GLuint idVBO;
		GLuint idIBO;

		std::string shaderProgram;
		glm::vec3 pos;

		bool visible = true;

	public:
		Model(std::string shaderProgram, const glm::vec3& pos = { 0, 0, 0 });

		Model(Model& orig);

		virtual ~Model();

		virtual void drawModel(glm::mat4 MVPMatrix) = 0;

		bool& getVisible();
	};
}


#endif //PAG_MODEL_H
