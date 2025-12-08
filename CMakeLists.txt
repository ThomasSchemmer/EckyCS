cmake_minimum_required(VERSION 4.0.2 FATAL_ERROR)

project(RailShooterCpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

# Put all compiled targets into one folder for easier use in the CLion IDE.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(OpenGL REQUIRED)
add_subdirectory(ext/glew-cmake)
add_subdirectory(ext/glfw)
add_subdirectory(ext/cmake-imgui)

include(FetchContent)
FetchContent_Declare(
        glm
        GIT_REPOSITORY	https://github.com/g-truc/glm.git
        GIT_TAG 	a532f5b1cf27d6a3c099437e6959cf7e398a0a67 #refs/tags/1.0.2
)
FetchContent_MakeAvailable(glm)

add_subdirectory(ImportsLib/src)


add_executable(RailShooterCpp
        src/main.cpp
        src/PlayerService.h
        src/HGame.cpp
        src/GAS/GE_DegenLife.cpp
        src/GAS/GA_SelfPoison.cpp
        src/GAS/AC_Player.cpp
        src/EckyCS/Systems/ItemRenderData.cpp
        src/EckyCS/Components/ItemComponent.h
        src/EckyCS/Entities/PlantTest.h
)

target_include_directories(RailShooterCpp
        PRIVATE
        src
        ImportsLib/src
        ${CMAKE_SOURCE_DIR}/ext
)
target_link_libraries(RailShooterCpp
        PRIVATE
        OpenGL::GL
        libglew_static
        glfw
        ImGui::imgui
        ImportsLib
        glm::glm
)
