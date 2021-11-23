#include "stdafx.h"
#include "RendererCore/Renderer.h"
#include "Interface/GUI.h"
#include "RendererCore/RenderOptions.h"

double prevXPos = 0, prevYPos = 0;
bool botonDerechoPulsado;
float deltaTime = 0.1f;
float ultimoFrame = 0.0f;

void actualizarDeltaTime() {
	const float frameActual = glfwGetTime();
	deltaTime = (frameActual - ultimoFrame) * 10;
	ultimoFrame = frameActual;
}

void GLAPIENTRY MessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam) {
	if (type == 0x824C) {
		fprintf(stderr, "GL DEBUG CALLBACK: ** GL ERROR ** type = 0x%x, severity = 0x%x, message = %s\n",
			type, severity, message);
	}
}

// - Esta función callback será llamada cada vez que el área de dibujo
// OpenGL deba ser redibujada.
void callbackRefrescoVentana(GLFWwindow* ventana) {
	try {
		PPCX::Renderer::getInstancia()->refrescar();
		GUI::getInstance()->render();
	} catch (std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
	}
	glfwSwapBuffers(ventana);
}


// - Esta función callback será llamada cada vez que se cambie el tamaño
// del área de dibujo OpenGL.
void callbackFramebufferSize(GLFWwindow* window, int width, int height) {
	PPCX::Renderer::getInstancia()->setViewport(0, 0, width, height);
}

// - Esta función callback será llamada cada vez que se pulse una tecla
// dirigida al área de dibujo OpenGL.
void callbackTecla(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	} else if (key == GLFW_KEY_W && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		PPCX::Renderer::getInstancia()->getCamara().truck(-1.0f * deltaTime);
	} else if (key == GLFW_KEY_S && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		PPCX::Renderer::getInstancia()->getCamara().truck(1.0f * deltaTime);
	} else if (key == GLFW_KEY_A && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		PPCX::Renderer::getInstancia()->getCamara().dolly(-1.0f * deltaTime);
	} else if (key == GLFW_KEY_D && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		PPCX::Renderer::getInstancia()->getCamara().dolly(1.0f * deltaTime);
	} else if (key == GLFW_KEY_Z && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		PPCX::Renderer::getInstancia()->getCamara().boom(1.0f * deltaTime);
	} else if (key == GLFW_KEY_X && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		PPCX::Renderer::getInstancia()->getCamara().crane(1.0f * deltaTime);
	} else if (key == GLFW_KEY_I && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		PPCX::Renderer::getInstancia()->getCamara().zoom(-4 * deltaTime);
	} else if (key == GLFW_KEY_O && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		PPCX::Renderer::getInstancia()->getCamara().zoom(4 * deltaTime);
	} else if (key == GLFW_KEY_Q && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		PPCX::Renderer::getInstancia()->getCamara().orbitX(50 * deltaTime);
	} else if (key == GLFW_KEY_E && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		PPCX::Renderer::getInstancia()->getCamara().orbitX(-50 * deltaTime);
	} else if (key == GLFW_KEY_T && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		PPCX::Renderer::getInstancia()->getCamara().orbitY(50 * deltaTime);
	} else if (key == GLFW_KEY_G && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		PPCX::Renderer::getInstancia()->getCamara().orbitY(-50 * deltaTime);
	} else if (key == GLFW_KEY_R && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		PPCX::Renderer::getInstancia()->getCamara().reset();
	}

}

// - Esta función callback será llamada cada vez que se pulse algún botón
// del ratón sobre el área de dibujo OpenGL.
void callbackBotonRaton(GLFWwindow* window, int button, int action, int mods) {
	ImGui::GetIO().MouseDown[button] = action == GLFW_PRESS;

	if (GUI::getInstance()->isMouseActive()) return;

	if (action == GLFW_PRESS) {
		if (button == 1) {
			botonDerechoPulsado = true;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
	} else if (action == GLFW_RELEASE) {
		if (button == 1) {
			botonDerechoPulsado = false;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}
}

void callbackMovimientoRaton(GLFWwindow* window, const double xpos, const double ypos) {
	if (!GUI::getInstance()->isMouseActive() && botonDerechoPulsado) {
		PPCX::Renderer::getInstancia()->getCamara().pan((xpos - prevXPos) * deltaTime * 10);
		PPCX::Renderer::getInstancia()->getCamara().tilt((ypos - prevYPos) * deltaTime * 10);
	}
	prevXPos = xpos;
	prevYPos = ypos;
}

void callbackScroll(GLFWwindow* window, double xoffset, double yoffset) {
	PPCX::Renderer::getInstancia()->getCamara().zoom(yoffset * deltaTime);
}

int main() {
	std::cout << "Starting application" << std::endl;
	// - Inicializa GLFW. Es un proceso que solo debe realizarse una vez en la aplicación
	if (glfwInit() != GLFW_TRUE) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4); // - Activa antialiasing con 4 muestras.
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // - Esta y las 2
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // siguientes activan un contexto
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1); // OpenGL Core Profile 4.1.

	// - Tamaño, título de la ventana, en ventana y no en pantalla completa,
	// sin compartir recursos con otras ventanas.
	GLFWwindow* window = glfwCreateWindow(PPCX::anchoVentanaPorDefecto, PPCX::altoVentanaPorDefecto,
		"PPCX: Procedural Point Cloud eXpander", nullptr, nullptr);
	// - Comprobamos si la creación de la ventana ha tenido éxito.
	if (window == nullptr) {
		std::cerr << "Failed to open GLFW window" << std::endl;
		glfwTerminate(); // - Liberamos los recursos que ocupaba GLFW.
		return -2;
	}

	glfwMakeContextCurrent(window);
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		std::cerr << "Failed to initialize GLEW" << std::endl;
		glfwDestroyWindow(window);
		glfwTerminate(); // - Liberamos los recursos que ocupaba GLFW.
		return -3;
	}

	//Realizamos esta llamada para forzar la creación del Renderer y asi poder capturar el posible error sin problema
	try {
		PPCX::Renderer::getInstancia();
	} catch (std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		std::cout << "Finishing application..." << std::endl;
		glfwDestroyWindow(window); // - Cerramos y destruimos la ventana de la aplicación.
		glfwTerminate(); // - Liberamos los recursos que ocupaba GLFW.
		return -1; //Forzamos el cierre si ha ocurrido alguna excepción en el shader
	}


	std::cout << PPCX::Renderer::getInstancia()->getPropiedadGL(GL_RENDERER) << std::endl
		<< PPCX::Renderer::getInstancia()->getPropiedadGL(GL_VENDOR) << std::endl
		<< PPCX::Renderer::getInstancia()->getPropiedadGL(GL_VERSION) << std::endl
		<< PPCX::Renderer::getInstancia()->getPropiedadGL(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	// - Registramos los callbacks que responderán a los eventos principales
	glfwSetWindowRefreshCallback(window, callbackRefrescoVentana);
	glfwSetFramebufferSizeCallback(window, callbackFramebufferSize);
	glfwSetKeyCallback(window, callbackTecla);
	glfwSetMouseButtonCallback(window, callbackBotonRaton);
	glfwSetScrollCallback(window, callbackScroll);
	glfwSetCursorPosCallback(window, callbackMovimientoRaton);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSwapInterval(1); // Enable vsync
	glDebugMessageCallback(MessageCallback, nullptr);


	PPCX::Renderer::getInstancia()->inicializaOpenGL();
	GUI::getInstance()->initialize(window, 1);

	// - Ciclo de eventos de la aplicación.
	while (!glfwWindowShouldClose(window)) {
		actualizarDeltaTime();
		glfwPollEvents();
		callbackRefrescoVentana(window);
	}

	std::cout << "Finishing application..." << std::endl;
	glfwDestroyWindow(window); // - Cerramos y destruimos la ventana de la aplicación.
	glfwTerminate(); // - Liberamos los recursos que ocupaba GLFW.
}