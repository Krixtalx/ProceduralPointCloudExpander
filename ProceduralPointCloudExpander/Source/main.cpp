// IMPORTANTE: El include de GLEW debe estar siempre ANTES de el de GLFW
#include "stdafx.h"
#include "RendererCore/Renderer.h"


int colorSeleccionado = 0;
double prevXPos = 0, prevYPos = 0;
bool botonDerechoPulsado;
std::string colores[3] = {"Rojo", "Verde", "Azul"};
float deltaTime = 0.1f;
float ultimoFrame = 0.0f;

void actualizarDeltaTime() {
	static clock_t ultimaEjecucion = clock(); // Solo se ejecuta la primera vez
	if ((clock() - ultimaEjecucion) > 25) {
		float frameActual = glfwGetTime();
		deltaTime = frameActual - ultimoFrame;
		ultimoFrame = frameActual;
		ultimaEjecucion = clock();
	}
}

void GLAPIENTRY MessageCallback(GLenum source,
                                GLenum type,
                                GLuint id,
                                GLenum severity,
                                GLsizei length,
                                const GLchar *message,
                                const void *userParam) {
	if (type == 0x824C) {
		fprintf(stderr, "GL DEBUG CALLBACK: ** GL ERROR ** type = 0x%x, severity = 0x%x, message = %s\n",
		        type, severity, message);
	}
}

// - Esta función callback será llamada cada vez que el área de dibujo
// OpenGL deba ser redibujada.
void callbackRefrescoVentana(GLFWwindow *ventana) {
	try {
		PAG::Renderer::getInstancia()->refrescar();
	} catch (std::runtime_error &e) {
		std::cerr << e.what() << std::endl;
	}
	glfwSwapBuffers(ventana);
	//std::cout << "Finaliza el callback de refresco" << std::endl;
}


// - Esta función callback será llamada cada vez que se cambie el tamaño
// del área de dibujo OpenGL.
void callbackFramebufferSize(GLFWwindow *window, int width, int height) {
	PAG::Renderer::getInstancia()->setViewport(0, 0, width, height);
	//std::cout << "Resize callback called" << std::endl;
}

// - Esta función callback será llamada cada vez que se pulse una tecla
// dirigida al área de dibujo OpenGL.
void callbackTecla(GLFWwindow *window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	} else if (key == GLFW_KEY_W && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		PAG::Renderer::getInstancia()->getCamara().truck(-1.0f * deltaTime);
	} else if (key == GLFW_KEY_S && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		PAG::Renderer::getInstancia()->getCamara().truck(1.0f * deltaTime);
	} else if (key == GLFW_KEY_A && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		PAG::Renderer::getInstancia()->getCamara().dolly(-1.0f * deltaTime);
	} else if (key == GLFW_KEY_D && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		PAG::Renderer::getInstancia()->getCamara().dolly(1.0f * deltaTime);
	} else if (key == GLFW_KEY_Z && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		PAG::Renderer::getInstancia()->getCamara().boom(1.0f * deltaTime);
	} else if (key == GLFW_KEY_X && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		PAG::Renderer::getInstancia()->getCamara().crane(1.0f * deltaTime);
	} else if (key == GLFW_KEY_I && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		PAG::Renderer::getInstancia()->getCamara().zoom(-4 * deltaTime);
	} else if (key == GLFW_KEY_O && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		PAG::Renderer::getInstancia()->getCamara().zoom(4 * deltaTime);
	} else if (key == GLFW_KEY_Q && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		PAG::Renderer::getInstancia()->getCamara().orbitX(50 * deltaTime);
	} else if (key == GLFW_KEY_E && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		PAG::Renderer::getInstancia()->getCamara().orbitX(-50 * deltaTime);
	} else if (key == GLFW_KEY_T && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		PAG::Renderer::getInstancia()->getCamara().orbitY(50 * deltaTime);
	} else if (key == GLFW_KEY_G && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		PAG::Renderer::getInstancia()->getCamara().orbitY(-50 * deltaTime);
	} else if (key == GLFW_KEY_R && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		PAG::Renderer::getInstancia()->getCamara().reset();
	}

	callbackRefrescoVentana(window);
	//std::cout << "Key callback called" << std::endl;
}

// - Esta función callback será llamada cada vez que se pulse algún botón
// del ratón sobre el área de dibujo OpenGL.
void callbackBotonRaton(GLFWwindow *window, int button, int action, int mods) {
	if (action == GLFW_PRESS) {
		if (button == 0) {
			colorSeleccionado = (colorSeleccionado + 1) % 3;
			std::cout << "Seleccionado el colorSeleccionado "
			          << colores[colorSeleccionado]
			          << std::endl;
		} else if (button == 1) {
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

void callbackMovimientoRaton(GLFWwindow *window, double xpos, double ypos) {
	if (botonDerechoPulsado) {
		PAG::Renderer::getInstancia()->getCamara().pan((xpos - prevXPos) * deltaTime * 10);
		PAG::Renderer::getInstancia()->getCamara().tilt((ypos - prevYPos) * deltaTime * 10);
		callbackRefrescoVentana(window);
	}
	prevXPos = xpos;
	prevYPos = ypos;
}

// - Esta función callback será llamada cada vez que se mueva la rueda
// del ratón sobre el área de dibujo OpenGL.
void callbackScroll(GLFWwindow *window, double xoffset, double yoffset) {
	float rojo = PAG::Renderer::getInstancia()->getRojoFondo();
	float verde = PAG::Renderer::getInstancia()->getVerdeFondo();
	float azul = PAG::Renderer::getInstancia()->getAzulFondo();
	//Cambio del colorSeleccionado en función al colorSeleccionado seleccionado. La selección se altera con el clic izquierdo del ratón
	if (colorSeleccionado == 0) {
		rojo += (float) (yoffset * 0.05f);
		if (rojo > 1) {
			rojo = 1;
		} else if (rojo < 0) {
			rojo = 0;
		}
	} else if (colorSeleccionado == 1) {
		verde += (float) (yoffset * 0.05f);
		if (verde > 1) {
			verde = 1;
		} else if (verde < 0) {
			verde = 0;
		}
	} else {
		azul += (float) (yoffset * 0.05f);
		if (azul > 1) {
			azul = 1;
		} else if (azul < 0) {
			azul = 0;
		}
	}

	std::cout << "rojoFondo: " << rojo << " verdeFondo: " << verde << " azulFondo: " << azul << std::endl;
	PAG::Renderer::getInstancia()->setColorFondo(rojo, verde, azul, 1);
	callbackRefrescoVentana(window);
}

int main() {
	std::cout << "Starting application" << std::endl;
	// - Inicializa GLFW. Es un proceso que solo debe realizarse una vez en la aplicación
	if (glfwInit() != GLFW_TRUE) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return -1;
	}

	// - Definimos las características que queremos que tenga el contexto gráfico
	// OpenGL de la ventana que vamos a crear. Por ejemplo, el número de muestras o el
	// modo Core Profile.
	glfwWindowHint(GLFW_SAMPLES, 4); // - Activa antialiasing con 4 muestras.
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // - Esta y las 2
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // siguientes activan un contexto
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1); // OpenGL Core Profile 4.1.

	// - Definimos el puntero para guardar la dirección de la ventana de la aplicación y
	// la creamos
	GLFWwindow *window;

	// - Tamaño, título de la ventana, en ventana y no en pantalla completa,
	// sin compartir recursos con otras ventanas.
	window = glfwCreateWindow(PAG::anchoVentanaPorDefecto, PAG::altoVentanaPorDefecto,
	                          "Procedural Point Cloud Expander", nullptr, nullptr);
	// - Comprobamos si la creación de la ventana ha tenido éxito.
	if (window == nullptr) {
		std::cerr << "Failed to open GLFW window" << std::endl;
		glfwTerminate(); // - Liberamos los recursos que ocupaba GLFW.
		return -2;
	}
	// - Hace que el contexto OpenGL asociado a la ventana que acabamos de crear pase a
	// ser el contexto actual de OpenGL para las siguientes llamadas a la biblioteca
	glfwMakeContextCurrent(window);

	// - Ahora inicializamos GLEW.
	// IMPORTANTE: GLEW debe inicializarse siempre DESPUÉS de que se haya
	// inicializado GLFW y creado la ventana
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		std::cerr << "Failed to initialize GLEW" << std::endl;
		glfwDestroyWindow(window);
		glfwTerminate(); // - Liberamos los recursos que ocupaba GLFW.
		return -3;
	}

	//Realizamos esta llamada para forzar la creación del Renderer y asi poder capturar el posible error sin problema
	try {
		PAG::Renderer::getInstancia();
	} catch (std::runtime_error &e) {
		std::cerr << e.what() << std::endl;
		std::cout << "Finishing application..." << std::endl;
		glfwDestroyWindow(window); // - Cerramos y destruimos la ventana de la aplicación.
		glfwTerminate(); // - Liberamos los recursos que ocupaba GLFW.
		return -1; //Forzamos el cierre si ha ocurrido alguna excepción en el shader
	}
	// - Interrogamos a OpenGL para que nos informe de las propiedades del contexto
	// 3D construido.
	std::cout << PAG::Renderer::getInstancia()->getPropiedadGL(GL_RENDERER) << std::endl
	          << PAG::Renderer::getInstancia()->getPropiedadGL(GL_VENDOR) << std::endl
	          << PAG::Renderer::getInstancia()->getPropiedadGL(GL_VERSION) << std::endl
	          << PAG::Renderer::getInstancia()->getPropiedadGL(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	// - Registramos los callbacks que responderán a los eventos principales
	glfwSetWindowRefreshCallback(window, callbackRefrescoVentana);
	glfwSetFramebufferSizeCallback(window, callbackFramebufferSize);
	glfwSetKeyCallback(window, callbackTecla);
	glfwSetMouseButtonCallback(window, callbackBotonRaton);
	glfwSetScrollCallback(window, callbackScroll);
	glfwSetCursorPosCallback(window, callbackMovimientoRaton);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glDebugMessageCallback(MessageCallback, 0);


	PAG::Renderer::getInstancia()->inicializaOpenGL();

	std::cout
			<< "Con el clic izquierdo del raton se selecciona el color a cambiar. Por defecto se encuentra el color rojo seleccionado."
			<< std::endl;
	std::cout << "Con la tecla W/S se realiza el movimiento truck" << std::endl;
	std::cout << "Con la tecla A/D se realiza el movimiento dolly" << std::endl;
	std::cout << "Con la tecla Z/X se realiza el movimiento boom/crane" << std::endl;
	std::cout << "Con la tecla I/O se realiza el zoom" << std::endl;
	std::cout << "Con la tecla Q/E se realiza el movimiento orbit horizontal" << std::endl;
	std::cout << "Con la tecla T/G se realiza el movimiento orbit vertical" << std::endl;
	std::cout << "Con los ejes del raton mientras se pulsa el boton derecho se realiza el movimiento pan y tilt"
	          << std::endl;
	std::cout << "Con la tecla R se resetea la camara a su posicion original" << std::endl;

	// - Ciclo de eventos de la aplicación. La condición de parada es que la
	// ventana principal deba cerrarse. Por ejemplo, si el usuario pulsa el
	// botón de cerrar la ventana (la X).
	while (!glfwWindowShouldClose(window)) {
		// - Obtiene y organiza los eventos pendientes, tales como pulsaciones de
		// teclas o de ratón, etc. Siempre al final de cada iteración del ciclo
		// de eventos y después de glfwSwapBuffers(window);
		glfwPollEvents();
		//actualizarDeltaTime();
	}
	// - Una vez terminado el ciclo de eventos, liberar recursos, etc.
	std::cout << "Finishing application..." << std::endl;
	glfwDestroyWindow(window); // - Cerramos y destruimos la ventana de la aplicación.
	glfwTerminate(); // - Liberamos los recursos que ocupaba GLFW.
}
