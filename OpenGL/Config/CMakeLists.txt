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
add_opengl_example(Project1-6 ../Project1/Project1-6.cpp Project1)
add_opengl_example(Project1-7 ../Project1/Project1-7.cpp Project1)
add_opengl_example(Project1-8 ../Project1/Project1-8.cpp Project1)
add_opengl_example(Project1-9 ../Project1/Project1-9.cpp Project1)
add_opengl_example(Project1-10 ../Project1/Project1-10.cpp Project1)
add_opengl_example(Project1-11 ../Project1/Project1-11.cpp Project1)
add_opengl_example(Project1-12 ../Project1/Project1-12.cpp Project1)
add_opengl_example(Project1-13 ../Project1/Project1-13.cpp Project1)
add_opengl_example(Project1-14 ../Project1/Project1-14.cpp Project1)
add_opengl_example(Project1-15 ../Project1/Project1-15.cpp Project1)
add_opengl_example(Project1-16 ../Project1/Project1-16.cpp Project1)
add_opengl_example(Project1-17 ../Project1/Project1-17.cpp Project1)
add_opengl_example(Project1-18 ../Project1/Project1-18.cpp Project1)
add_opengl_example(Project1-19 ../Project1/Project1-19.cpp Project1)
# 실행 위치와 관계없이 프로젝트의 OBJ 폴더를 찾도록 경로를 전달한다.
foreach(example Project1-17 Project1-18 Project1-19)
    target_compile_definitions(${example} PRIVATE
        OBJ_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/../Project1/models"
    )
endforeach()

# 16번은 아직 작성 중이므로 전체 빌드에서 제외한다. 구현 후 이 설정을 제거한다.
set_target_properties(Project1-16 PROPERTIES
    EXCLUDE_FROM_ALL TRUE
    EXCLUDE_FROM_DEFAULT_BUILD TRUE
)
