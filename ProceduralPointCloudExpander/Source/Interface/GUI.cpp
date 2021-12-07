#include "stdafx.h"
#include "GUI.h"
#include "Interface/Fonts/lato.hpp"
#include "Interface/Fonts/IconsFontAwesome5.h"
#include "imfiledialog/ImGuiFileDialog.h"
#include "RendererCore/Renderer.h"


/// [Protected methods]

GUI::GUI() :
	_showAboutUs(false), _showControls(false), _showFileDialog(false),
	_showPointCloudDialog(false), _showRenderingSettings(false) {

}

void GUI::createMenu() {
	const ImGuiIO& io = ImGui::GetIO();

	if (_showRenderingSettings)		showRenderingSettings();
	if (_showAboutUs)				showAboutUsWindow();
	if (_showControls)				showControls();
	if (_showFileDialog)			showFileDialog();
	if (_showPointCloudDialog)		showPointCloudDialog();

	if (ImGui::BeginMainMenuBar()) {
		ImGui::MenuItem(ICON_FA_SAVE "Open Point Cloud", nullptr, &_showFileDialog);
		if (sceneLoaded)
			ImGui::MenuItem(ICON_FA_CUBE "Rendering", nullptr, &_showRenderingSettings);

		if (ImGui::BeginMenu(ICON_FA_SITEMAP "Procedural Options")) {
			ImGui::EndMenu();
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
	ImGui::TextDisabled(ICON_FA_QUESTION);
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
	if (ImGui::Begin("Scene controls", &_showControls, ImGuiWindowFlags_NoCollapse)) {
		ImGui::Columns(2, "ControlColumns"); // 4-ways, with border
		ImGui::Separator();
		ImGui::Text("Movement"); ImGui::NextColumn();
		ImGui::Text("Control"); ImGui::NextColumn();
		ImGui::Separator();

		const int NUM_MOVEMENTS = 12;
		const char* movement[] = { "Orbit (XZ)", "Orbit (Y)", "Truck", "Dolly", "Boom", "Crane", "Reset Camera", "Take Screenshot", "Zoom +/-", "Pan", "Tilt", "Increase/Decrease zFar" };
		const char* controls[] = { "Move mouse horizontally(hold left button)", "Move mouse vertically (hold left button)", "W, S", "A, D", "Z", "X", "R", "K", "Scroll wheel", "Move mouse horizontally(hold right button)", "Move mouse vertically (hold right button)", "+/-" };

		for (int i = 0; i < NUM_MOVEMENTS; i++) {
			ImGui::Text(movement[i]); ImGui::NextColumn();
			ImGui::Text(controls[i]); ImGui::NextColumn();
		}

		ImGui::Columns(1);
		ImGui::Separator();

	}

	ImGui::End();
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

void GUI::showPointCloudDialog() {
	if (ImGui::Begin("Open Point Cloud", &_showPointCloudDialog, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking)) {
		bool newScene = false;
		this->leaveSpace(1);

		ImGui::Checkbox("New Scene", &newScene);

		this->leaveSpace(2);

		if (ImGui::Button("Open Point Cloud")) {
			PPCX::Renderer::getInstancia()->cargaModelo(_pointCloudPath, newScene);
			_showPointCloudDialog = false;
			sceneLoaded = true;
		}

	}

	ImGui::End();
}

void GUI::showRenderingSettings() {
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
	ImGui::SetNextWindowBgAlpha(0.6f);
	if (ImGui::Begin("Rendering Settings", &_showRenderingSettings, window_flags)) {
		glm::vec3 color = PPCX::Renderer::getInstancia()->getColorFondo();
		ImGui::ColorEdit3("Background color", &color[0]);
		PPCX::Renderer::getInstancia()->setColorFondo(color);
		this->leaveSpace(3);

		if (ImGui::BeginTabBar("")) {
			if (ImGui::BeginTabItem("Point Cloud")) {
				this->leaveSpace(1);
				float value = PPCX::Renderer::getInstancia()->getPointSize();
				ImGui::SliderFloat("Point size", &value, 0.1f, 10.0f);
				PPCX::Renderer::getInstancia()->setPointSize(value);

				ImGui::Checkbox("Original cloud", &PPCX::Renderer::getInstancia()->getPointCloudVisible(0));
				ImGui::Checkbox("NURBS cloud", &PPCX::Renderer::getInstancia()->getPointCloudVisible(1));
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Statistics")) {
				this->leaveSpace(1);
				ImGui::Text("Number of points loaded: ");
				ImGui::Text("Number of points in original point cloud: ");
				ImGui::Text("Number of points in nurbs point cloud: ");
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
	ImGui::End();
}


GUI::~GUI() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

/// [Public methods]

void GUI::initialize(GLFWwindow* window, const int openGLMinorVersion) {
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

	this->createDockspace();
	this->createMenu();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

// ---------------- IMGUI ------------------

void GUI::loadImGUIStyle() {
	ImGui::StyleColorsDark();
	this->loadStyle();
	this->loadFonts();
}

void GUI::loadFonts() {
	ImFontConfig cfg;
	ImGuiIO& io = ImGui::GetIO();

	io.Fonts->AddFontFromFileTTF("Assets/Fonts/Ruda-Bold.ttf", 14.0f);

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
	style->Colors[ImGuiCol_Border] = ImVec4(0.40f, 0.40f, 0.43f, 0.88f);
	style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.52f, 0.51f, 0.58f, 0.00f);
	style->Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
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
