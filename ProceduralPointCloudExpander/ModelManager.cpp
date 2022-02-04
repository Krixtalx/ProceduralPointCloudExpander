#include "stdafx.h"
#include "RendererCore/ModelManager.h"

ModelManager* ModelManager::instance = nullptr;

ModelManager* ModelManager::getInstance() {
	if (!instance)
		instance = new ModelManager();
	return instance;
}

void ModelManager::newModel(const std::string& key, PPCX::Model* model) {
	models.insert(std::make_pair(key, model));
}

PPCX::Model* ModelManager::getModel(const std::string& key) {
	const auto model = models.find(key);
	if(model == models.end())
		throw std::runtime_error("[ModelManager::getModel]: Cannot find any model with that key.");
	return model->second;
}

std::vector<std::pair<std::string, PPCX::Model*>> ModelManager::getAllModels() {
	std::vector<std::pair<std::string, PPCX::Model*>> vec(models.begin(), models.end());
	return vec;
}
