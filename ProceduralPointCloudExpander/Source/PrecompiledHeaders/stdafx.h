#define _USE_MATH_DEFINES

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
#include <random>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <thread>

// [Standard libraries: data structures]

#include <map>
#include <vector>
#include <set>
#include <unordered_map>
#include <unordered_set>