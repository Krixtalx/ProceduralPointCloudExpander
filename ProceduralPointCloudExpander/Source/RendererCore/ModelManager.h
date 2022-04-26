#pragma once
#include "PointCloudHQRRenderer.h"
#include "RendererCore/Model.h"
#include "Utilities/Singleton.h"

class ModelManager : public Singleton<ModelManager> {
	std::map<std::string, PPCX::Model*> models;
	std::unique_ptr<PointCloudHQRRenderer> hqrRenderer;
	std::vector<std::pair<std::string, PointCloud*>> pendingClouds;
public:
	bool hqrRendering = true;
	ModelManager();
	~ModelManager();
	void drawModels(const glm::mat4& matrizMVP);
	void drawAndDeleteSingleModel(const std::string& modelKey, const glm::mat4& matrizMVP);

	void newModel(const std::string& key, PPCX::Model* model);
	void modifyModel(const std::string& key, PPCX::Model* model);
	void deleteModel(const std::string& key);

	PPCX::Model* getModel(const std::string& key);
	std::vector<std::pair<std::string, PPCX::Model*>> getAllModels();
	std::vector<std::pair<std::string, PPCX::Model*>> getAllModelsLike(const std::string& key) const;

	void setAllVisibility(bool visible) const;
	void setVisibility(const std::string& key, bool visible) const;

	void exportAllVisibleModels(const std::string& filename) const;
	void updateWindowSize(glm::vec2 newWindowSize);
};
