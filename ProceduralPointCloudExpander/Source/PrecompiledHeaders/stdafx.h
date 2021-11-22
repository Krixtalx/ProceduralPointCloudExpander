#define _USE_MATH_DEFINES
#define GLM_ENABLE_EXPERIMENTAL

// [Libraries]

#include "GL/glew.h"								// Don't swap order between GL and GLFW includes!
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/ext/vector_relational.hpp"
#include "glm/gtx/string_cast.hpp"
#include <glm/gtc/type_ptr.hpp>

// [Image]

#include "lodepng.h"

// [ImGui]
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

// [Standard libraries: basic]

#include <algorithm>
#include <cmath>
#include <chrono>
#include <cstdint>
#include <execution>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <numeric>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <thread>

#include <omp.h>

// [Standard libraries: data structures]

#include <map>
#include <vector>
#include <set>
#include <unordered_map>
#include <unordered_set>