#include "stdafx.h"
#include "ModelManager.h"

#include "ComputeShader.h"
#include "pointCloud.h"
#include "Utilities/FileManager.h"


ModelManager::ModelManager() {}

ModelManager::~ModelManager() {
	for (const auto& model : models) {
		delete model.second;
	}
}

/**
 * Includes a new model in the ModelManager. By default, if the model visibility is not changed, it will appear in the rendering.
 *
 * @param key Key that will identify the model in the ModelManager
 * @param model Model itself
 */
void ModelManager::newModel(const std::string& key, PPCX::Model* model) {
	models.insert(std::make_pair(key, model));
	auto cloud = dynamic_cast<PointCloud*>(model);
	if (cloud) {
		pendingClouds.emplace_back(key, cloud);
		if (cloud->classification == "High vegetation" && key != "High vegetation") {
			generatedVegetation.insert(key);
			generatedCloudsName.insert(key);
		}
	}
}

/**
 * Replace a model in the ModelManager.
 *
 * @param key Key that will identify the model in the ModelManager
 * @param model The new model that will replace the existing model (If anyone)
 */
void ModelManager::modifyModel(const std::string& key, PPCX::Model* model) {
	deleteModel(key);
	newModel(key, model);
}

/**
 * Delete a model in the ModelManager
 *
 * @param key Key of the model in the ModelManager
 */
void ModelManager::deleteModel(const std::string& key) {
	if (const auto model = models.find(key); model != models.end()) {
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

std::vector<std::pair<std::string, PPCX::Model*>> ModelManager::getAllModelsLike(const std::string& key) const {
	std::vector<std::pair<std::string, PPCX::Model*>> vec;
	for (const auto& model : models) {
		if (model.first.find(key) != std::string::npos)
			vec.emplace_back(model);
	}
	return vec;
}

std::set<std::string>& ModelManager::getVegetationClouds() {
	return generatedVegetation;
}

/**
 * Method used by the renderer to draw all the models in the ModelManager
 *
 * @param matrizMVP Model view perspective view matrix.
 */
void ModelManager::drawModels(const mat4& matrizMVP) const {
	for (const auto& model : models) {
		model.second->drawModel(matrizMVP);
	}
}

void ModelManager::drawAndDeleteSingleModel(const std::string& modelKey, const mat4& matrizMVP) {
	try {
		const auto model = getModel(modelKey);
		model->drawModel(matrizMVP);
		deleteModel(modelKey);
	} catch (std::runtime_error& e) {
	}
}

void ModelManager::setAllVisibility(const bool visible) const {
	for (const auto& model : models) {
		model.second->getVisibility() = visible;
	}
}

void ModelManager::setVisibility(const std::string& key, const bool visible) const {
	for (auto model : models) {
		if (model.first.find(key) != std::string::npos)
			model.second->getVisibility() = visible;
	}
}

void ModelManager::exportAllVisibleModels(const std::string& filename) const {
	std::vector<PointCloud*> clouds;
	for (const auto& model : models) {
		if (model.second->getVisibility()) {
			if (auto* cloud = dynamic_cast<PointCloud*>(model.second)) {
				clouds.push_back(cloud);
			}
		}
	}
	std::thread thread(&FileManager::savePointCloud, filename, clouds);
	thread.detach();
	//FileManager::savePointCloud(filename, clouds);
}

unsigned ModelManager::getNumberOfPoints() {
	unsigned n = 0;
	for (const auto& model : models) {
		if (model.second->getVisibility()) {
			if (const auto* cloud = dynamic_cast<PointCloud*>(model.second)) {
				n += cloud->getNumberOfPoints();
			}
		}
	}
	return n;
}
