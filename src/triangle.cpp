#include <map>
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

    this->validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

#ifdef NDEBUG
    this->validationLayersEnabled = false;
#else
    this->validationLayersEnabled = true;
#endif
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

    // Set up the debug layer
    setupDebugMessenger();

    // Select the physical Device
    pickPhysicalDevice();

    // Create a logical device based on the physical devices
    createLogicalDevice();
}

void TriangleApplication::createVkInstance() {
    // Enumerate available extensions
    uint32_t vkInstanceExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &vkInstanceExtensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(vkInstanceExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &vkInstanceExtensionCount, availableExtensions.data());

    std::cout << "Available extensions:" << std::endl;

    for (const auto& extension : availableExtensions) {
        std::cout << "\t" << extension.extensionName << std::endl;
    }

    std::cout.flush();

    // Check for validation layer support
    if (this->validationLayersEnabled && !checkValidationLayerSupport()){
        throw std::runtime_error("Validation layers were requested, but they are unavailable.");
    }

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
    
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (this->validationLayersEnabled){
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    }
    else {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    // Get GLFW extensions
    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

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

    // Clean up the logical device
    vkDestroyDevice(this->device, nullptr);
    
    // Clean up debug messenger
    if (this->validationLayersEnabled){
        destroyVkDebugMessenger();
    }
    vkDestroyInstance(this->vkInstance, nullptr);

    glfwDestroyWindow(this->window);

    glfwTerminate();
}

bool TriangleApplication::checkValidationLayerSupport(){
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layer : this->validationLayers){
        bool layerFound = false;

        for (const auto& layerProperty : availableLayers){
            if (strcmp(layer, layerProperty.layerName) == 0){
                layerFound = true;
                break;
            }
        }

        if (!layerFound){
            return false;
        }
    }

    return true;
}

std::vector<const char*> TriangleApplication::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (this->validationLayersEnabled){
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

void TriangleApplication::populateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT& createInfo
){
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT     |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;
}

void TriangleApplication::pickPhysicalDevice(){
    this->physicalDevice = VK_NULL_HANDLE;

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(this->vkInstance, &deviceCount, nullptr);

    if (deviceCount == 0){
        throw std::runtime_error("Failed to find any Vulkan-Compatible GPUs.");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(this->vkInstance, &deviceCount, devices.data());

    std::multimap<unsigned int, VkPhysicalDevice> candidates;
    for (const auto& device : devices){
        unsigned int score = rateDeviceSuitability(device);
        candidates.insert(std::make_pair(score, device));
    }

    if (candidates.rbegin()->first > 0){
        this->physicalDevice = candidates.rbegin()->second;
    }
    else {
        throw std::runtime_error("Failed to find suitable GPU.");
    }
}

bool TriangleApplication::QueueFamilyIndicies::isComplete(){
    return graphicsFamily.has_value();
}

unsigned int TriangleApplication::rateDeviceSuitability(const VkPhysicalDevice device){
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;

    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    unsigned int score = 0;

    QueueFamilyIndicies indicies = findQueueFamilies(device);
    if(!indicies.isComplete()) return 0;

    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        score += 1000;
    
    score += deviceProperties.limits.maxImageDimension2D;

    std::cout << "Device: " << deviceProperties.deviceName << std::endl << 
        "\tScore: " << score << std::endl;

    return score;
}

TriangleApplication::QueueFamilyIndicies TriangleApplication::findQueueFamilies(VkPhysicalDevice device){
    QueueFamilyIndicies indicies = {};

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies){
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT){
            indicies.graphicsFamily = i;
        }

        if (indicies.isComplete()) break;

        i++;
    }

    return indicies;
}

void TriangleApplication::setupDebugMessenger(){
    if (!this->validationLayersEnabled) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    auto result = createVkDebugMessenger(
        &createInfo,
        nullptr,
        &this->debugMessenger
    );

    if (result != VK_SUCCESS){
        throw std::runtime_error("Failed to set up debug messenger.");
    }
}

VkResult TriangleApplication::createVkDebugMessenger(
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger
){
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
        this->vkInstance,
        "vkCreateDebugUtilsMessengerEXT");
    
    if (func != nullptr){
        return func(
            this->vkInstance,
            pCreateInfo,
            pAllocator,
            pDebugMessenger
        );
    }
    else{
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void TriangleApplication::createLogicalDevice(){
    QueueFamilyIndicies indicies = findQueueFamilies(physicalDevice);

    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indicies.graphicsFamily.value();
    queueCreateInfo.queueCount = 1;

    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures = {}; // Everything is still VK_FALSE but we'll fix that later

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pEnabledFeatures = &deviceFeatures;

    if (this->validationLayersEnabled) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(this->validationLayers.size());
        createInfo.ppEnabledLayerNames = this->validationLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

    VkResult deviceCreationResult = vkCreateDevice(
        this->physicalDevice,
        &createInfo,
        nullptr,
        &this->device
    );

    if (deviceCreationResult != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device");
    }
}

void TriangleApplication::destroyVkDebugMessenger(
    const VkAllocationCallbacks* pAllocator
){
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
        this->vkInstance,
        "vkDestroyDebugUtilsMessengerEXT"
    );

    if (func != nullptr){
        func(this->vkInstance, this->debugMessenger, pAllocator);
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL TriangleApplication::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
){
    std::cerr << "Validation Layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}