#include "stdafx.h"
#include "ModelManager.h"

ModelManager* ModelManager::instance = nullptr;

ModelManager::~ModelManager() {
	for (const auto& model : models) {
		delete model.second;
	}
}

ModelManager* ModelManager::getInstance() {
	if (!instance)
		instance = new ModelManager();
	return instance;
}

/**
 * Includes a new model in the ModelManager. By default, if the model visibility is not changed, it will appear in the rendering.
 *
 * @param key Key that will identify the model in the ModelManager
 * @param model Model itself
 */
void ModelManager::newModel(const std::string& key, PPCX::Model* model) {
	models.insert(std::make_pair(key, model));
}

/**
 * Delete a model in the ModelManager
 *
 * @param key Key of the model in the ModelManager
 */
void ModelManager::deleteModel(const std::string& key) {
	const auto model = models.find(key);
	if (model != models.end()) {
		delete model->second;
		models.erase(key);
	}
}

/**
 * Gets a pointer to the model.
 *
 * @param key Key of the model in the ModelManager
 * @return Pointer to the model indicated by the key
 */
PPCX::Model* ModelManager::getModel(const std::string& key) {
	const auto model = models.find(key);
	if (model == models.end())
		throw std::runtime_error("[ModelManager::getModel]: Cannot find any model with that key.");
	return model->second;
}

/**
 * Get a vector containing all the models
 *
 * @return a vector containing all the pairs in the ModelManager models map (Key and value)
 */
std::vector<std::pair<std::string, PPCX::Model*>> ModelManager::getAllModels() {
	std::vector<std::pair<std::string, PPCX::Model*>> vec(models.begin(), models.end());
	return vec;
}

/**
 * Method used by the renderer to draw all the models in the ModelManager
 * 
 * @param matrizMVP Model view perspective view matrix.
 */
void ModelManager::drawModels(const glm::mat4& matrizMVP) {
	for (auto& model : models) {
		model.second->drawModel(matrizMVP);
	}
}
