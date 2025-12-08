add_subdirectory(src)

add_library(ImportsLib)
target_link_libraries(ImportsLib
        PUBLIC
        EckyCS
        GameService
        GAS
        Player
        Renderer
        Terrain
        Util
)