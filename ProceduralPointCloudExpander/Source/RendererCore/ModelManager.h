#pragma once
#include "RendererCore/Model.h"

class ModelManager {
private:
	static ModelManager* instance;
	ModelManager() = default;

	std::map<std::string, PPCX::Model*> models;

public:
	~ModelManager();
	static ModelManager* getInstance();
	void drawModels(const glm::mat4& matrizMVP) const;
	void drawAndDeleteSingleModel(const std::string& modelKey, const glm::mat4& matrizMVP);

	void newModel(const std::string& key, PPCX::Model* model);
	void modifyModel(const std::string& key, PPCX::Model* model);
	void deleteModel(const std::string& key);

	PPCX::Model* getModel(const std::string& key);
	std::vector<std::pair<std::string, PPCX::Model*>> getAllModels();
	std::vector<std::pair<std::string, PPCX::Model*>> getAllModelsLike(const std::string& key);

	void setAllVisibility(bool visible);
	void setVisibility(std::string key, bool visible);
};