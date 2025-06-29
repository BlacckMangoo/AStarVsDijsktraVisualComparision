cmake_minimum_required(VERSION 3.8)

project(Raytracing)

# SDL3 and SDL3_image subdirectories
add_subdirectory(${CMAKE_SOURCE_DIR}/SDL)

# Add source files
add_executable(raytracing "main.cpp")

# Add includes
target_include_directories(raytracing PRIVATE
    ${CMAKE_SOURCE_DIR}/SDL/include
)

# Link SDL3 and SDL_image
target_link_libraries(raytracing SDL3-shared)

# Copy SDL DLL on Windows
if(WIN32)
    add_custom_command(TARGET raytracing POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL3-shared>
        $<TARGET_FILE_DIR:raytracing>
    )
endif()

# C++20
if (CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET raytracing PROPERTY CXX_STANDARD 20)
endif()
