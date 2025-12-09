#include "GA_SelfPoison.h"

#include <iostream>
#include <ostream>

#include "GameService/Game.h"

bool GA_SelfPoison::ShouldActivate()
{
    if (!GameplayAbility::ShouldActivate())
        return false;

    return glfwGetKey(GameImports::Game::Instance->WindowPtr, GLFW_KEY_X) == GLFW_PRESS;
}

void GA_SelfPoison::OnActivate()
{
    GameplayAbility::OnActivate();

    cout << "GA_SelfPoison::OnActivate" << endl;
}

