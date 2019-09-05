#include <vector>
#include <iostream>
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
    initWindow();
    initVulkan();
    mainLoop();
    cleanUp();
}

void TriangleApplication::initVulkan() {
    // Enter initialization code here

    // Create a vulkan instance
    createVkInstance();
}

void TriangleApplication::createVkInstance() {
    // Enumerate available extensions
    uint32_t vkInstanceExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &vkInstanceExtensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(vkInstanceExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &vkInstanceExtensionCount, extensions.data());

    std::cout << "Available extensions:" << std::endl;

    for (const auto& extension : extensions) {
        std::cout << "\t" << extension.extensionName << std::endl;
    }

    std::cout.flush();

    // Give Vulkan some information (for optimization later)
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = this->title.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_1;

    // Create the information struct
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = 0;

    // Get GLFW extensions
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;

    // Create the Vulkan instance and check status
    VkResult result = vkCreateInstance(&createInfo, nullptr, &this->vkInstance);

    if (result != VK_SUCCESS){
        throw std::runtime_error("Could not create Vulkan Instance");
    }
}

void TriangleApplication::initWindow() {
    // Initialize the glfw window
    glfwInit();

    // Configure the glfw window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // No OpenGL Context
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);   // Not resizeable

    // Create the window instance
    this->window = glfwCreateWindow(
        this->initialWindowWidth,
        this->initialWindowHeight,
        this->title.c_str(),
        nullptr,
        nullptr
    );
}

void TriangleApplication::mainLoop() {
    // Enter main loop code here
    while (!glfwWindowShouldClose(this->window)){
        glfwPollEvents();
    }
}

void TriangleApplication::cleanUp() {
    // Enter clean up code here
    vkDestroyInstance(this->vkInstance, nullptr);

    glfwDestroyWindow(this->window);

    glfwTerminate();
}