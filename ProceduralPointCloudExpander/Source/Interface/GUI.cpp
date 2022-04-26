#include "stdafx.h"
#include "GUI.h"

#include "imgui_internal.h"
#include "Interface/Fonts/IconsFontAwesome5.h"
#include "imfiledialog/ImGuiFileDialog.h"
#include "RendererCore/Renderer.h"
#include "Utilities/Loader.h"
#include <RendererCore/ModelManager.h>


/// [Protected methods]

GUI::GUI() :
	_showAboutUs(false), _showControls(false), _showFileDialog(false),
	_showSaveDialog(false), _showPointCloudDialog(false), _showRenderingSettings(false), _showProceduralSettings(false), _showVegetationSettings(false),
	procGenerator(PPCX::Renderer::getInstancia()->getProceduralGenerator()) {}

void GUI::createMenu() {
	const ImGuiIO& io = ImGui::GetIO();

	if (_showRenderingSettings && procGenerator->progress >= 10000.0f)		showRenderingSettings();
	if (_showProceduralSettings && procGenerator->progress >= 10000.0f)		showProceduralSettings();
	if (_showAboutUs)				showAboutUsWindow();
	if (_showControls)				showControls();
	if (_showFileDialog)			showFileDialog();
	if (_showSaveDialog)			showSaveWindow();
	if (_showPointCloudDialog)		showPointCloudDialog();
	if (_showVegetationSettings && procGenerator->progress >= 10000.0f)		showVegetationSelection();

	if (ImGui::BeginMainMenuBar()) {
		if (Loader::saving)
			Spinner("Saving Spinner", 8, 4, ImGui::GetColorU32(ImGuiCol_ButtonHovered));
		if (ImGui::BeginMenu(ICON_FA_FILE "File")) {
			ImGui::MenuItem(ICON_FA_FOLDER_OPEN "Open point cloud", nullptr, &_showFileDialog);
			if (sceneLoaded && procGenerator->progress >= 1.0f) {
				if (ImGui::MenuItem(ICON_FA_SAVE "Save point cloud", nullptr, &_showSaveDialog))
					saveOption = 0;
				if (ImGui::MenuItem(ICON_FA_SAVE "Save height map", nullptr, &_showSaveDialog))
					saveOption = 1;
				if (ImGui::MenuItem(ICON_FA_SAVE "Save texture map", nullptr, &_showSaveDialog))
					saveOption = 2;
			}
			ImGui::EndMenu();
		}
		if (sceneLoaded && procGenerator->progress >= 1.0f) {
			ImGui::MenuItem(ICON_FA_CUBE "Rendering", nullptr, &_showRenderingSettings);
			ImGui::MenuItem(ICON_FA_SITEMAP "Procedural settings", nullptr, &_showProceduralSettings);
		}

		ImGui::SameLine();
		ImGui::SetCursorPosX(io.DisplaySize.x - 130);
		ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
		if (ImGui::BeginMenu(ICON_FA_QUESTION_CIRCLE "Help")) {
			ImGui::MenuItem(ICON_FA_INFO "About the project", nullptr, &_showAboutUs);
			ImGui::MenuItem(ICON_FA_GAMEPAD "Controls", nullptr, &_showControls);
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}

void GUI::leaveSpace(const unsigned numSlots) {
	for (int i = 0; i < numSlots; ++i) {
		ImGui::Spacing();
	}
}

void GUI::renderHelpMarker(const char* message) {
	ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4(0.85f, 0.85f, 0.93f, 1.00f));
	ImGui::TextDisabled(ICON_FA_QUESTION);
	ImGui::PopStyleColor();
	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(message);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void GUI::showAboutUsWindow() {
	if (ImGui::Begin("About the project", &_showAboutUs, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking)) {
		ImGui::Text("This code is developed by Jose Antonio Collado Araque as a part of a research project from University of Jaen (GGGJ group).");
	}

	ImGui::End();
}

void GUI::showControls() {
	ImGui::SetNextWindowBgAlpha(0.6f);
	if (ImGui::Begin("Scene controls", &_showControls, ImGuiWindowFlags_NoCollapse)) {
		if (ImGui::BeginTable("ControlsTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders)) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Text("     Movement"); ImGui::TableNextColumn();
			ImGui::Text("     Control");

			constexpr int NUM_MOVEMENTS = 13;
			const char* movement[] = { "Orbit (XZ)", "Orbit (Y)", "Truck", "Dolly", "Boom", "Crane", "Reset Camera", "Take Screenshot", "Zoom +/-", "Pan", "Tilt", "Increase/Decrease zFar", "Change camara type" };
			const char* controls[] = { "Move mouse horizontally(hold left button)", "Move mouse vertically (hold left button)", "W, S", "A, D", "Z", "X", "R", "K", "Scroll wheel", "Move mouse horizontally(hold right button)", "Move mouse vertically (hold right button)", "+/-", "P" };

			for (int i = 0; i < NUM_MOVEMENTS; i++) {
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text(movement[i]);
				ImGui::TableNextColumn();
				ImGui::Text(controls[i]);
			}
			ImGui::EndTable();
		}
		ImGui::End();
	}
}

void GUI::showFileDialog() {
	ImGuiFileDialog::Instance()->OpenDialog("Choose Point Cloud", "Choose File", ".ply", ".");

	// display
	if (ImGuiFileDialog::Instance()->Display("Choose Point Cloud")) {
		// action if OK
		if (ImGuiFileDialog::Instance()->IsOk()) {
			const std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			_pointCloudPath = filePathName.substr(0, filePathName.find_last_of('.'));
			_showPointCloudDialog = true;
		}

		// close
		ImGuiFileDialog::Instance()->Close();
		_showFileDialog = false;
	}
}

void GUI::showSaveWindow() {
	if (saveOption == 0)
		ImGuiFileDialog::Instance()->OpenDialog("Choose save location", "Save", ".ply", ".DefaultName");
	else
		ImGuiFileDialog::Instance()->OpenDialog("Choose save location", "Save", ".png", ".DefaultName");
	// display
	if (ImGuiFileDialog::Instance()->Display("Choose save location")) {
		// action if OK
		if (ImGuiFileDialog::Instance()->IsOk()) {
			_pointCloudPath = ImGuiFileDialog::Instance()->GetFilePathName();
			if (saveOption == 0)
				ModelManager::getInstance()->exportAllVisibleModels(_pointCloudPath);
			else if (saveOption == 1)
				procGenerator->saveHeightMap(_pointCloudPath);
			else if (saveOption == 2)
				procGenerator->saveTextureMap(_pointCloudPath);
		}

		// close
		ImGuiFileDialog::Instance()->Close();
		_showSaveDialog = false;
	}
}

void GUI::showPointCloudDialog() {
	if (ImGui::Begin("Open Point Cloud", &_showPointCloudDialog, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking)) {
		static bool newScene = false;
		this->leaveSpace(1);

		//ImGui::Checkbox("New Scene", &newScene);
		ImGui::Text("Desired points per voxel: ");
		ImGui::SliderInt("##pointPerVoxel", &pointsPerVoxel, 1, 400);

		this->leaveSpace(2);

		if (ImGui::Button("Open Point Cloud")) {
			procGenerator->progress = .0f;
			std::thread thread(&PPCX::Renderer::cargaModelo, PPCX::Renderer::getInstancia(), _pointCloudPath, newScene, pointsPerVoxel);
			thread.detach();
			_showPointCloudDialog = false;
			sceneLoaded = true;
			newScene = false;
		}
		ImGui::End();
	}

}

void GUI::showRenderingSettings() {
	constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
	ImGui::SetNextWindowBgAlpha(0.6f);
	if (ImGui::Begin("Rendering Settings", &_showRenderingSettings, window_flags)) {
		vec3 color = PPCX::Renderer::getInstancia()->getColorFondo();
		ImGui::ColorEdit3("Background color", &color[0]);
		PPCX::Renderer::getInstancia()->setColorFondo(color);
		this->leaveSpace(3);
		ImGui::Checkbox("High quality rendering", &ModelManager::getInstance()->hqrRendering);
		this->leaveSpace(3);

		if (ImGui::BeginTabBar("")) {
			if (ImGui::BeginTabItem("Point Cloud")) {
				this->leaveSpace(1);
				float value = PPCX::Renderer::getInstancia()->getPointSize();
				ImGui::SliderFloat("Point size", &value, 0.1f, 10.0f);
				PPCX::Renderer::getInstancia()->setPointSize(value);

				ImGui::Text("Loaded clouds");
				for (unsigned i = 0; i < Loader::LASClassificationSize; ++i) {
					try {
						PPCX::Model* model = ModelManager::getInstance()->getModel(Loader::LASClassificationStrings[i]);
						ImGui::Checkbox(Loader::LASClassificationStrings[i].c_str(), &model->getVisibility());
					} catch (std::runtime_error& e) {
					}
				}

				ImGui::Text("Generated clouds");
				for (auto& cloudName : ProceduralGenerator::generatedCloudsName) {
					try {
						PPCX::Model* model = ModelManager::getInstance()->getModel(cloudName);
						ImGui::Checkbox(cloudName.c_str(), &model->getVisibility());
					} catch (std::runtime_error& e) {
					}
				}

				ImGui::Separator();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Statistics")) {
				if (ImGui::BeginTable("table1", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit)) {
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::Text("Number of points loaded"); ImGui::TableNextColumn();
					if (procGenerator->terrainCloud)
						ImGui::Text("%i", procGenerator->terrainCloud->getNumberOfPoints());
					ImGui::TableNextRow(); ImGui::TableNextColumn();

					ImGui::Text("Number of subdivisions in x"); ImGui::TableNextColumn();
					ImGui::Text("%i", procGenerator->axisSubdivision[0]);
					ImGui::TableNextRow(); ImGui::TableNextColumn();

					ImGui::Text("Number of subdivisions in y"); ImGui::TableNextColumn();
					ImGui::Text("%i", procGenerator->axisSubdivision[1]);
					ImGui::TableNextRow(); ImGui::TableNextColumn();

					ImGui::Text("Cloud density"); ImGui::TableNextColumn();
					ImGui::Text("%f", procGenerator->cloudDensity);
					ImGui::TableNextRow(); ImGui::TableNextColumn();

					ImGui::Text("Cloud X size"); ImGui::TableNextColumn();
					ImGui::Text("%f", procGenerator->aabb.size().x);
					ImGui::TableNextRow(); ImGui::TableNextColumn();

					ImGui::Text("Cloud Y size"); ImGui::TableNextColumn();
					ImGui::Text("%f", procGenerator->aabb.size().y);
					//ImGui::TableNextRow(); ImGui::TableNextColumn();

					ImGui::EndTable();
				}
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
	ImGui::End();
}

void GUI::showProceduralSettings() {
	constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;

	ImGui::SetNextWindowBgAlpha(0.8f);
	if (ImGui::Begin("Procedural Settings", &_showProceduralSettings, window_flags)) {
		if (ImGui::BeginTabBar("")) {
			if (ImGui::BeginTabItem("NURBS")) {
				ImGui::Text("Nurbs degree: ");
				ImGui::SliderInt("##degree", &degree, 2, 8);
				ImGui::Checkbox("Generate points based on internal point distribution", &procGenerator->useStatiticsMethod);
				ImGui::SameLine(); renderHelpMarker("[Experimental]: The points generated from the NURBS will be generated based on the current internal distribution of the voxel. This means that there will be more probability generating points in empty spaces of each voxel");
				if (procGenerator->useStatiticsMethod) {
					ImGui::Text("Internal subdivisions: ");
					ImGui::InputInt("##internalSubDiv", &internalSubdivision, 1, 1);
				}
				ImGui::Text("Density multiplier: ");
				ImGui::SliderFloat("##densityMultiplier", &densityMultiplier, 1.0f, 10.0f, "%.1f");
				ImGui::SameLine(); renderHelpMarker("Multiplier used to get the desired cloud density. Using a value of 1, you will generate points only in voxels that don't have enought points (Empty voxels or voxels with big holes for example)");
				this->leaveSpace(1);
				if (ImGui::Button("Generate nurbs cloud")) {
					ModelManager::getInstance()->deleteModel("Nurbs generated cloud");
					std::thread thread(&ProceduralGenerator::computeNURBS, procGenerator, degree, internalSubdivision, internalSubdivision, densityMultiplier);
					thread.detach();
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Voxel grid")) {
				ImGui::Text("Desired points per voxel: ");
				ImGui::SliderInt("##pointPerVoxel", &pointsPerVoxel, 5, 400);
				this->leaveSpace(1);
				if (ImGui::Button("Generate voxel grid")) {
					std::thread thread(&ProceduralGenerator::generateVoxelGrid, procGenerator, pointsPerVoxel);
					thread.detach();
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("RGB Region segmentation")) {
				static float distanceThreshold = 10.0f, colorThreshold = 6.0f, regionColorThreshold = 4.0f;
				static int minClusterSize = 25000;
				ImGui::InputFloat("Distance threshold", &distanceThreshold, 0.1f, 1.0f, "%.1f");
				ImGui::InputFloat("Point color threshold", &colorThreshold, 0.1f, 1.0f, "%.1f");
				ImGui::InputFloat("Region color threshold", &regionColorThreshold, 0.1f, 1.0f, "%.1f");
				ImGui::InputInt("Min cluster size", &minClusterSize);
				if (minClusterSize < 1)
					minClusterSize = 1;
				if (ImGui::Button("Generate RGB region segmentation")) {
					std::thread thread(&ProceduralGenerator::RegionRGBSegmentation, procGenerator, distanceThreshold, colorThreshold, regionColorThreshold, minClusterSize);
					thread.detach();
					_showVegetationSettings = true;
				}
				/*if (ImGui::Button("Test RGB region segmentation")) {
					std::thread thread(&ProceduralGenerator::testRGBSegmentation, procGenerator);
					thread.detach();
				}*/
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::End();
	}

}

void GUI::showVegetationSelection() {
	if (ImGui::Begin("Vegetation settings", &_showVegetationSettings, ImGuiWindowFlags_NoCollapse)) {
		ModelManager::getInstance()->setAllVisibility(false);
		const auto models = ModelManager::getInstance()->getAllModelsLike("RGB");
		const char* items[] = { "OliveTree.ply", "PineTree.ply" };
		static std::vector<int> values;

		while (values.size() < models.size()) {
			values.push_back(0);
		}

		int i = 0;
		for (const auto& model : models) {
			const auto cloud = dynamic_cast<PointCloud*>(model.second);
			cloud->getVisibility() = true;
			const auto color = cloud->getRandomPointColor();
			ImGui::ColorButton(model.first.c_str(), ImVec4(color.r / 256, color.g / 256, color.b / 256, 1), ImGuiColorEditFlags_NoBorder, ImVec2(20, 20));
			ImGui::SameLine();
			ImGui::Text(model.first.c_str());
			ImGui::SameLine();
			ImGui::Combo(("##Picker Mode" + std::to_string(i)).c_str(), &values[i++], "OliveTree.ply\0PineTree.ply\0", 2);
		}
		if (ImGui::Button("Generate procedural vegetation")) {
			std::vector<std::pair<std::string, std::string>> vec;
			vec.reserve(models.size());
			for (int j = 0; j < models.size(); ++j) {
				vec.emplace_back(models[j].first, items[values[j]]);
			}
			procGenerator->generateProceduralVegetation(vec);
			ModelManager::getInstance()->setAllVisibility(true);
			ModelManager::getInstance()->setVisibility("RGB", false);
			ModelManager::getInstance()->setVisibility("High", false);
			_showVegetationSettings = false;
		}
		ImGui::End();
	}
}

void GUI::showProgressBar() const {
	if (procGenerator->progress < 100.0f) {
		if (ImGui::Begin("Progress", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse)) {
			if (procGenerator->progress < .25f)
				ImGui::Text("Loading point cloud...");
			else if (procGenerator->progress < .45f)
				ImGui::Text("Creating voxel grid...");
			else if (procGenerator->progress < .55f)
				ImGui::Text("Computing height and colors...");
			else if (procGenerator->progress < .65f)
				ImGui::Text("Creating nurbs...");
			else if (procGenerator->progress < .8f)
				ImGui::Text("Generating nurbs cloud...");
			else if (procGenerator->progress < 1.0f)
				ImGui::Text("Finishing...");
			else if (procGenerator->progress < 1.3f)
				ImGui::Text("Starting clustering process...");
			else if (procGenerator->progress < 1.6f)
				ImGui::Text("Computing the clusters...");
			else if (procGenerator->progress < 1.8f)
				ImGui::Text("Generating colored clusters clouds...");
			else if (procGenerator->progress < 2.0f)
				ImGui::Text("Adding clusters clouds to the renderer...");

			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.32f, 0.39f, 0.87f, 1.00f));
			ImGui::ProgressBar(procGenerator->progress - (int)procGenerator->progress);
			ImGui::PopStyleColor();
			ImGui::End();
		}
	}
}


GUI::~GUI() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}



/// [Public methods]

void GUI::initialize(GLFWwindow* window, const int openGLMinorVersion) const {
	const std::string openGLVersion = "#version 4" + std::to_string(openGLMinorVersion) + "0 core";

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	this->loadImGUIStyle();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(openGLVersion.c_str());

}

void GUI::render() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	createDockspace();
	this->createMenu();
	this->showProgressBar();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

// ---------------- IMGUI ------------------

void GUI::loadImGUIStyle() const {
	ImGui::StyleColorsDark();
	loadStyle();
	this->loadFonts();
}

void GUI::loadFonts() const {
	ImFontConfig cfg;
	ImGuiIO& io = ImGui::GetIO();

	io.Fonts->AddFontFromFileTTF("Assets/Fonts/Roboto-Bold.ttf", 14.0f);

	static constexpr ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	cfg.MergeMode = true;
	cfg.PixelSnapH = true;
	cfg.GlyphMinAdvanceX = 20.0f;
	cfg.GlyphMaxAdvanceX = 20.0f;
	std::copy_n("FontAwesome", 12, cfg.Name);

	io.Fonts->AddFontFromFileTTF("Assets/Fonts/fa-regular-400.ttf", 13.0f, &cfg, icons_ranges);
	io.Fonts->AddFontFromFileTTF("Assets/Fonts/fa-solid-900.ttf", 13.0f, &cfg, icons_ranges);
	io.Fonts->Build();
}


void GUI::loadStyle() {
	ImGuiStyle* style = &ImGui::GetStyle();

	style->WindowPadding = ImVec2(15, 15);
	style->WindowRounding = 5.0f;
	style->FramePadding = ImVec2(5, 5);
	style->FrameRounding = 4.0f;
	style->ItemSpacing = ImVec2(12, 8);
	style->ItemInnerSpacing = ImVec2(8, 6);
	style->IndentSpacing = 25.0f;
	style->ScrollbarSize = 15.0f;
	style->ScrollbarRounding = 9.0f;
	style->GrabMinSize = 5.0f;
	style->GrabRounding = 3.0f;

	style->Colors[ImGuiCol_Text] = ImVec4(0.85f, 0.85f, 0.93f, 1.00f);
	style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_ChildBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_Border] = ImVec4(0.10f, 0.10f, 0.13f, 0.88f);
	style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.52f, 0.51f, 0.58f, 0.00f);
	style->Colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.19f, 0.22f, 1.00f);
	style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.34f, 0.33f, 0.39f, 1.00f);
	style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
	style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_CheckMark] = ImVec4(0.32f, 0.39f, 0.87f, 1.00f);
	style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.22f, 0.29f, 0.67f, 1.00f);
	style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.32f, 0.38f, 0.77f, 1.00f);
	style->Colors[ImGuiCol_Button] = ImVec4(0.22f, 0.29f, 0.67f, 1.00f);
	style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.32f, 0.75f, 1.00f);
	style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.32f, 0.38f, 0.77f, 1.00f);
	style->Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
	style->Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);
}

void GUI::createDockspace() {
	ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
}

bool GUI::Spinner(const char* label, float radius, int thickness, const ImU32& color) {
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);

	ImVec2 pos = window->DC.CursorPos;
	ImVec2 size((radius) * 2, (radius + style.FramePadding.y) * 2);

	const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
	ImGui::ItemSize(bb, style.FramePadding.y);
	if (!ImGui::ItemAdd(bb, id))
		return false;

	// Render
	window->DrawList->PathClear();

	int num_segments = 30;
	int start = abs(ImSin(g.Time * 1.8f) * (num_segments - 5));

	const float a_min = IM_PI * 2.0f * ((float)start) / (float)num_segments;
	const float a_max = IM_PI * 2.0f * ((float)num_segments - 3) / (float)num_segments;

	const ImVec2 centre = ImVec2(pos.x + radius, pos.y + radius + style.FramePadding.y);

	for (int i = 0; i < num_segments; i++) {
		const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
		window->DrawList->PathLineTo(ImVec2(centre.x + ImCos(a + g.Time * 8) * radius,
											centre.y + ImSin(a + g.Time * 8) * radius));
	}

	window->DrawList->PathStroke(color, false, thickness);
}