#pragma once

#include "Utilities/Singleton.h"
#include "ProceduralUtils/ProceduralGenerator.h"

/**
*	@file GUI.h
*	@authors Alfonso L�pez Ruiz (alr00048@red.ujaen.es), Jos� Antonio Collado Araque (jaca0011@red.ujaen.es)
*/

/**
*	@brief Wrapper for GUI graphics which allows the user to interact with the scene.
*/
class GUI final : public Singleton<GUI> {
	friend class Singleton<GUI>;

protected:

	// GUI state
	std::string						_pointCloudPath;					//!<
	bool							_showAboutUs;						//!< About us window
	bool							_showControls;						//!< Shows application controls
	bool							_showFileDialog;					//!< Shows a file dialog that allows opening a point cloud in .ply format
	bool							_showPointCloudDialog;				//!< 
	bool							_showRenderingSettings;				//!< Displays a window which allows the user to modify the rendering parameters
	bool							_showProceduralSettings;
	bool							sceneLoaded = false;
	ProceduralGenerator* procGenerator = nullptr;

protected:
	/**
	*	@brief Constructor of GUI context provided by a graphics library (Dear ImGui).
	*/
	GUI();

	/**
	*	@brief Creates the navbar.
	*/
	void createMenu();

	/**
	*	@brief Calls ImGui::Spacing() for n times in a clean way.
	*/
	static void leaveSpace(const unsigned numSlots);

	/**
	*	@brief
	*/
	//void loadFonts();

	/**
	*	@brief
	*/
	void loadImGUIStyle();

	/**
	*	@brief Renders a help icon with a message.
	*/
	static void renderHelpMarker(const char* message);

	/**
	*	@brief Shows a window where some information about the project is displayed.
	*/
	void showAboutUsWindow();

	/**
	*	@brief Displays a table with the application controls (mouse, keyboard, etc).
	*/
	void showControls();

	/**
	*	@brief Displays a file dialog to open a new point cloud (.ply).
	*/
	void showFileDialog();

	/**
	*	@brief
	*/
	void showPointCloudDialog();

	/**
	*	@brief Shows a window with general rendering configuration.
	*/
	void showRenderingSettings();

	void showProceduralSettings();

	void showProgressBar();

	void loadStyle();

	void createDockspace();

public:
	/**
	*	@brief Destructor.
	*/
	virtual ~GUI();

	/**
	*	@brief Initializes the context provided by an interface library.
	*	@param window Window provided by GLFW.
	*	@param openGLMinorVersion Minor version of OpenGL 4.
	*/
	void initialize(GLFWwindow* window, const int openGLMinorVersion);

	/**
	*	@brief Renders the interface components.
	*/
	void render();

	void loadFonts();

	bool isMouseActive() { return ImGui::GetIO().WantCaptureMouse;}
};