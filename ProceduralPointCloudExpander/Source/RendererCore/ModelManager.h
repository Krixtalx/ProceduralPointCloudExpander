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
	void newModel(const std::string& key, PPCX::Model* model);
	void deleteModel(const std::string& key);
	PPCX::Model* getModel(const std::string& key);
	std::vector<std::pair<std::string, PPCX::Model*>> getAllModels();
};