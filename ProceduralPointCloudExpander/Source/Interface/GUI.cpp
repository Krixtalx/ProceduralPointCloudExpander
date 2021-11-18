#include "stdafx.h"
#include "GUI.h"
#include "Interface/Fonts/lato.hpp"
#include "Interface/Fonts/IconsFontAwesome5.h"
#include "imfiledialog/ImGuiFileDialog.h"
#include "RendererCore/Renderer.h"


/// [Protected methods]

GUI::GUI() :
	_showAboutUs(false), _showControls(false), _showFileDialog(false),
	_showPointCloudDialog(false), _showRenderingSettings(false)
{

}

void GUI::createMenu()
{
	const ImGuiIO& io = ImGui::GetIO();

	if (_showRenderingSettings)		showRenderingSettings();
	if (_showAboutUs)				showAboutUsWindow();
	if (_showControls)				showControls();
	if (_showFileDialog)			showFileDialog();
	if (_showPointCloudDialog)		showPointCloudDialog();

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu(ICON_FA_COG "Settings"))
		{
			ImGui::MenuItem(ICON_FA_CUBE "Rendering", nullptr, &_showRenderingSettings);
			ImGui::MenuItem(ICON_FA_SAVE "Open Point Cloud", nullptr, &_showFileDialog);
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu(ICON_FA_QUESTION_CIRCLE "Help"))
		{
			ImGui::MenuItem(ICON_FA_INFO "About the project", nullptr, &_showAboutUs);
			ImGui::MenuItem(ICON_FA_GAMEPAD "Controls", nullptr, &_showControls);
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu(ICON_FA_SITEMAP "Procedural Options"))
		{
			ImGui::MenuItem(ICON_FA_INFO "About the project", nullptr, &_showAboutUs);
			ImGui::EndMenu();
		}

		ImGui::SameLine();
		ImGui::SetCursorPosX(io.DisplaySize.x - 125);
		GUI::renderHelpMarker("Avoids some movements to also modify the camera parameters");

		ImGui::SameLine(0, 20);
		ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
		ImGui::EndMainMenuBar();
	}
}

void GUI::leaveSpace(const unsigned numSlots)
{
	for (int i = 0; i < numSlots; ++i)
	{
		ImGui::Spacing();
	}
}

void GUI::renderHelpMarker(const char* message)
{
	ImGui::TextDisabled(ICON_FA_QUESTION);
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(message);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void GUI::showAboutUsWindow()
{
	if (ImGui::Begin("About the project", &_showAboutUs))
	{
		ImGui::Text("This code belongs to a research project from University of Jaen (GGGJ group).");
	}

	ImGui::End();
}

void GUI::showControls()
{
	if (ImGui::Begin("Scene controls", &_showControls))
	{
		ImGui::Columns(2, "ControlColumns"); // 4-ways, with border
		ImGui::Separator();
		ImGui::Text("Movement"); ImGui::NextColumn();
		ImGui::Text("Control"); ImGui::NextColumn();
		ImGui::Separator();

		const int NUM_MOVEMENTS = 14;
		const char* movement[] = { "Orbit (XZ)", "Undo Orbit (XZ)", "Orbit (Y)", "Undo Orbit (Y)", "Dolly", "Truck", "Boom", "Crane", "Reset Camera", "Take Screenshot", "Continue Animation", "Zoom +/-", "Pan", "Tilt" };
		const char* controls[] = { "X", "Ctrl + X", "Y", "Ctrl + Y", "W, S", "D, A", "Up arrow", "Down arrow", "R", "K", "I", "Scroll wheel", "Move mouse horizontally(hold button)", "Move mouse vertically (hold button)" };

		for (int i = 0; i < NUM_MOVEMENTS; i++)
		{
			ImGui::Text(movement[i]); ImGui::NextColumn();
			ImGui::Text(controls[i]); ImGui::NextColumn();
		}

		ImGui::Columns(1);
		ImGui::Separator();

	}

	ImGui::End();
}

void GUI::showFileDialog()
{
	ImGuiFileDialog::Instance()->OpenDialog("Choose Point Cloud", "Choose File", ".ply", ".");

	// display
	if (ImGuiFileDialog::Instance()->Display("Choose Point Cloud"))
	{
		// action if OK
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			const std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			_pointCloudPath = filePathName.substr(0, filePathName.find_last_of('.'));
			_showPointCloudDialog = true;
		}

		// close
		ImGuiFileDialog::Instance()->Close();
		_showFileDialog = false;
	}
}

void GUI::showPointCloudDialog()
{
	if (ImGui::Begin("Open Point Cloud Dialog", &_showPointCloudDialog))
	{
		static bool aggregate;
		this->leaveSpace(1);

		ImGui::Text("Open point cloud");
		ImGui::Separator();

		this->leaveSpace(1);

		ImGui::Checkbox("Aggregate", &aggregate);

		ImGui::PushID(0);
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(1 / 7.0f, 0.6f, 0.6f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(1 / 7.0f, 0.7f, 0.7f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(1 / 7.0f, 0.8f, 0.8f));

		this->leaveSpace(2);

		if (ImGui::Button("Open Point Cloud"))
		{
			PPCX::Renderer::getInstancia()->cargaModelo(_pointCloudPath);
			_showPointCloudDialog = false;
		}

		ImGui::PopStyleColor(3);
		ImGui::PopID();
	}

	ImGui::End();
}

void GUI::showRenderingSettings()
{
	if (ImGui::Begin("Rendering Settings", &_showRenderingSettings)) {
		glm::vec3 color = PPCX::Renderer::getInstancia()->getColorFondo();
		ImGui::ColorEdit3("Background color", &color[0]);
		PPCX::Renderer::getInstancia()->setColorFondo(color);

		this->leaveSpace(3);

		if (ImGui::BeginTabBar("LiDARTabBar"))
		{
			if (ImGui::BeginTabItem("General settings"))
			{
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Point Cloud"))
			{
				this->leaveSpace(1);

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Wireframe"))
			{
				this->leaveSpace(1);

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}
	ImGui::End();
}


GUI::~GUI()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

/// [Public methods]

void GUI::initialize(GLFWwindow* window, const int openGLMinorVersion)
{
	const std::string openGLVersion = "#version 4" + std::to_string(openGLMinorVersion) + "0 core";

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	this->loadImGUIStyle();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(openGLVersion.c_str());
}

void GUI::render()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	this->createMenu();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	
}

// ---------------- IMGUI ------------------

void GUI::loadImGUIStyle()
{
	ImGui::StyleColorsDark();

	this->loadFonts();
}

void GUI::loadFonts()
{
	ImFontConfig cfg;
	ImGuiIO& io = ImGui::GetIO();
	
	std::copy_n("Lato", 5, cfg.Name);
	io.Fonts->AddFontFromMemoryCompressedBase85TTF(lato_compressed_data_base85, 15.0f, &cfg);

	static constexpr ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	cfg.MergeMode = true;
	cfg.PixelSnapH = true;
	cfg.GlyphMinAdvanceX = 20.0f;
	cfg.GlyphMaxAdvanceX = 20.0f;
	std::copy_n("FontAwesome", 12, cfg.Name);

	io.Fonts->AddFontFromFileTTF("Assets/Fonts/fa-regular-400.ttf", 13.0f, &cfg, icons_ranges);
	io.Fonts->AddFontFromFileTTF("Assets/Fonts/fa-solid-900.ttf", 13.0f, &cfg, icons_ranges);
}