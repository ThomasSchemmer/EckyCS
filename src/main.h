#pragma once

#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "../ext/imgui/imgui.h"
#include "PlayerService.h"
#include "GameService/Game.h"

/**
 * Main entry point of the system, contains main().
 * Contains the actual game instance from which any service is launched,
 * as well as render and debug instances
 */

class Renderer;
using namespace std;
using namespace GameImports;

inline unique_ptr<Renderer> RendererPtr;