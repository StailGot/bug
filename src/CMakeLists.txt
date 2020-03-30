cmake_minimum_required(VERSION 3.16)

project(textures VERSION 0.0.1 LANGUAGES CXX)

file( GLOB_RECURSE SRC
    *.cpp
    # *.c
    *.h
    *.hpp
 )

add_executable(textures ${SRC})

set_target_properties(textures PROPERTIES
    CXX_STANDARD 20
    CXX_EXTENSIONS OFF
)

target_link_libraries(textures glfw glad)

target_include_directories(textures 
    PRIVATE
    "."
)