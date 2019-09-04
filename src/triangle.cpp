#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <triangle.hpp>

using namespace triangle;

TriangleApplication::TriangleApplication(
    std::string title,
    int initialWidth,
    int initialHeight
) {
    this->title = title;
    
    this->initialWindowWidth = initialWidth;
    this->initialWindowHeight = initialHeight;
}

void TriangleApplication::run() {
    initVulkan();
    mainLoop();
    cleanUp();
}

void TriangleApplication::initVulkan() {
    // Enter initialization code here
}

void TriangleApplication::initWindow() {
    // Initialize the glfw window
    glfwInit();

    // Configure the glfw window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // No OpenGL Context
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);   // Not resizeable

    // Create the window instance
    GLFWwindow* window = nullptr;
    window = glfwCreateWindow(
        this->initialWindowWidth,
        this->initialWindowHeight,
        this->title.c_str(),
        nullptr,
        nullptr
    );


}

void TriangleApplication::mainLoop() {
    // Enter main loop code here
}

void TriangleApplication::cleanUp() {
    // Enter clean up code here
}