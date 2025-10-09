#pragma once

#ifndef INCLUDE_GUARD_MAIN
#define INCLUDE_GUARD_MAIN


#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "../ext/imgui/imgui.h"
#include "Imports/GameService/Game.h"
#include "Imports/GameService/GameService.h"
#include "PlayerService.h"
#include "Imports/Renderer/Renderer.h"

using namespace std;
using namespace GameImports;

inline Game* GamePtr;
inline PlayerService* PlayerPtr;
inline Renderer* RendererPtr;
#endif // !INCLUDE_GUARD_MAIN