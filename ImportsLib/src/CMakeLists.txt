add_subdirectory(EckyCS)

add_library(GameService
        GameService/Game.cpp
        GameService/Game.h
        GameService/GameService.cpp
        GameService/GameService.h
        GameService/GameServiceDelegate.cpp
        GameService/GameServiceDelegate.h
)

add_subdirectory(GAS)

add_library(Player
        Player/PlayerController.cpp
        Player/PlayerController.h
)

add_library(Renderer
        Renderer/BaseShader.cpp
        Renderer/BaseShader.h
        Renderer/Camera.cpp
        Renderer/Camera.h
        Renderer/Renderer.cpp
        Renderer/Renderer.h
        Renderer/Utils.h
)
target_include_directories(Renderer
        PUBLIC
        ${CMAKE_SOURCE_DIR}/ext
)
target_link_libraries(Renderer
        PUBLIC
        libglew_static
        glfw
        glm::glm
)

add_library(Terrain
        Terrain/TerrainData.cpp
        Terrain/TerrainData.h
        Terrain/TerrainManager.cpp
        Terrain/TerrainManager.h
        Terrain/TerrainShader.cpp
        Terrain/TerrainShader.h
)

add_library(Util
        Util/ActionMap.h
        Util/Headers.h
        Util/ShaderHelper.cpp
)
target_include_directories(
        Util
        PUBLIC
        ${CMAKE_SOURCE_DIR}/ext
)