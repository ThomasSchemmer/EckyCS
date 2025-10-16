#pragma once

#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "../ext/imgui/imgui.h"
#include "Imports/GameService/Game.h"
#include "Imports/GameService/GameService.h"
#include "PlayerService.h"
#include "Imports/Renderer/Renderer.h"

/**
 * Main entry point of the system, contains main().
 * Contains the actual game instance from which any service is launched,
 * as well as render and debug instances
 */

using namespace std;
using namespace GameImports;

inline unique_ptr<Renderer> RendererPtr;