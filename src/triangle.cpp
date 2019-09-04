#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <triangle.hpp>

using namespace triangle;

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
}

void TriangleApplication::mainLoop() {
    // Enter main loop code here
}

void TriangleApplication::cleanUp() {
    // Enter clean up code here
}