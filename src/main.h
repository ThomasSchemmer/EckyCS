#pragma once

#include <glew/include/GL/glew.h>
#include "GLFW/include/GLFW/glfw3.h"
#include <renderdoc/renderdoc_app.h>

/**
 * Main entry point of the system, contains main().
 * Contains the actual game instance from which any service is launched,
 * as well as render and debug instances
 */

inline GLFWwindow* Window;
inline RENDERDOC_API_1_1_2* RDocAPI = nullptr;