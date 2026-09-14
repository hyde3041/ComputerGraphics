cmake_minimum_required(VERSION 3.25)

project(OpenGLPractices LANGUAGES CXX)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")

function(add_opengl_example target source_file solution_folder)
    add_executable(${target} ${source_file})
    target_compile_features(${target} PRIVATE cxx_std_20)
    target_link_libraries(${target} PRIVATE
        OpenGL::GL
        GLEW::GLEW
        glfw
        glm::glm
    )
    set_property(TARGET ${target} PROPERTY FOLDER ${solution_folder})

    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive- /utf-8)
    endif()
endfunction()

add_opengl_example(Project1-1 ../Project1/Project1-1.cpp Project1)
add_opengl_example(Project1-2 ../Project1/Project1-2.cpp Project1)
add_opengl_example(Project1-3 ../Project1/Project1-3.cpp Project1)
add_opengl_example(Project1-4 ../Project1/Project1-4.cpp Project1)
add_opengl_example(Project1-5 ../Project1/Project1-5.cpp Project1)
add_opengl_example(Project2-1 ../Project2/Project2-1.cpp Project2)
add_opengl_example(Project2-2 ../Project2/Project2-2.cpp Project2)
