cmake_minimum_required(VERSION 3.7)
project(vulkan-triangle VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

include(CTest)
enable_testing()

find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)

include_directories(
    include/
)

add_executable(vulkan-triangle 
    src/triangle.cpp
    src/main.cpp
)

target_link_libraries( vulkan-triangle
    glfw
    glm
    Vulkan::Vulkan
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
