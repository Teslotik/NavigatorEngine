cmake_minimum_required(VERSION 3.17)

project(engine)

set(CMAKE_CXX_STANDARD 20)  # @todo 20

include_directories(
    modules/glad/include

    modules/glfw/include

    modules/stb

    # modules/pocketpy/include
    modules/json/include

    modules

    engine
)

set(SOURCE
    engine/main.cpp
    engine/Application.cpp
    engine/Loader.cpp
    engine/utils.cpp
    engine/Easing.cpp

    engine/entity/Tag.cpp

    modules/glad/glad.c
    modules/stb/stb_image.c

    engine/script/StreetMap.cpp
)

enable_testing()

add_subdirectory(modules/glfw)
add_subdirectory(modules/glm)
add_subdirectory(tests)

add_executable(engine ${SOURCE})
target_link_libraries(engine ${OPENGL_LIBRARIES} glfw)

add_custom_command(TARGET engine POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/user"
    "${CMAKE_BINARY_DIR}/user"
)