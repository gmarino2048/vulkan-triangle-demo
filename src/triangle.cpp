#include <map>
#include <set>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <triangle.hpp>

#define VK_STD_VALIDATION_LAYERS "VK_LAYER_KHRONOS_validation"

using namespace triangle;

TriangleApplication::TriangleApplication(
    std::string title,
    int initialWidth,
    int initialHeight
) {
    this->title = title;
    
    this->initialWindowWidth = initialWidth;
    this->initialWindowHeight = initialHeight;

    this->deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    this->validationLayers = {
        VK_STD_VALIDATION_LAYERS
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

    // Create the Vulkan Surface
    createSurface();

    // Select the physical Device
    pickPhysicalDevice();

    // Create a logical device based on the physical devices
    createLogicalDevice();

    // Create the display swapchain
    createSwapChain();

    // Create the swapchain image views
    createImageViews();

    // Create the render pass
    createRenderPass();

    // Create the graphics pipeline
    createGraphicsPipeline();

    // Create Framebuffers
    createFrameBuffers();

    // Create the drawing command pool
    createCommandPool();

    // Create the command buffers
    createCommandBuffers();
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

    // Clean up the command pool
    vkDestroyCommandPool(this->device, this->commandPool, nullptr);

    // Clean up the framebuffers
    for (auto framebuffer : this->swapChainFramebuffers){
        vkDestroyFramebuffer(this->device, framebuffer, nullptr);
    }

    // Clean up the graphics pipeline
    vkDestroyPipeline(this->device, this->graphicsPipeline, nullptr);

    // Clean up the graphics pipeline layout
    vkDestroyPipelineLayout(this->device, this->pipelineLayout, nullptr);

    // Clean up the render pass
    vkDestroyRenderPass(this->device, this->renderPass, nullptr);

    // Clean up the image views
    for (auto imageView : this->swapChainImageViews){
        vkDestroyImageView(
            this->device,
            imageView,
            nullptr
        );
    }

    // Destroy the swapchain
    vkDestroySwapchainKHR(this->device, this->swapChain, nullptr);

    // Clean up the logical device
    vkDestroyDevice(this->device, nullptr);
    
    // Clean up the surface instance
    vkDestroySurfaceKHR(this->vkInstance, this->surface, nullptr);

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

void TriangleApplication::createSurface(){
    VkResult surfaceCreationResult = glfwCreateWindowSurface(
        this->vkInstance,
        this->window,
        nullptr,
        &this->surface
    );

    if (surfaceCreationResult != VK_SUCCESS){
        throw std::runtime_error("Could not create window surface");
    }
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
    return graphicsFamily.has_value() && presentFamily.has_value();
}

bool TriangleApplication::checkDeviceExtensionSupport(const VkPhysicalDevice device){
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(
        this->deviceExtensions.begin(),
        this->deviceExtensions.end()
    );

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

unsigned int TriangleApplication::rateDeviceSuitability(const VkPhysicalDevice device){
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;

    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    unsigned int score = 0;

    QueueFamilyIndicies indicies = findQueueFamilies(device);
    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = 
            !swapChainSupport.formats.empty() &&
            !swapChainSupport.presentModes.empty();
    }

    if(!indicies.isComplete() ||
        !extensionsSupported  ||
        !swapChainAdequate) 
        return 0;

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

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, this->surface, &presentSupport);

        if (queueFamily.queueCount > 0 && presentSupport){
            indicies.presentFamily = i;
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

    std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        indicies.graphicsFamily.value(),
        indicies.presentFamily.value()
    };

    float queuePriority = 1.0f;
    for (auto queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        deviceQueueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {}; // Everything is still VK_FALSE but we'll fix that later

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(deviceQueueCreateInfos.size());
    createInfo.pQueueCreateInfos = deviceQueueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(this->deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = this->deviceExtensions.data();

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

    vkGetDeviceQueue(
        this->device,
        indicies.graphicsFamily.value(),
        0,
        &this->graphicsQueue
    );

    vkGetDeviceQueue(
        this->device,
        indicies.presentFamily.value(),
        0,
        &this->presentQueue
    );
}


void TriangleApplication::createSwapChain() {
    SwapChainSupportDetails swapchainDetails = querySwapChainSupport(this->physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapchainDetails.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapchainDetails.presentModes);
    VkExtent2D swapExtent = chooseSwapExtent(swapchainDetails.capabilities);

    uint32_t imageCount = swapchainDetails.capabilities.minImageCount + 1;

    if (swapchainDetails.capabilities.maxImageCount > 0 &&
        imageCount > swapchainDetails.capabilities.maxImageCount){
            imageCount = swapchainDetails.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = this->surface;

    swapchainCreateInfo.minImageCount = imageCount;
    swapchainCreateInfo.imageFormat = surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent = swapExtent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndicies indicies = findQueueFamilies(this->physicalDevice);
    uint32_t queueFamilyIndicies[] = {
        indicies.graphicsFamily.value(),
        indicies.presentFamily.value()
    };

    if (indicies.graphicsFamily != indicies.presentFamily){
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndicies;
    }
    else {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.queueFamilyIndexCount = 0;
        swapchainCreateInfo.pQueueFamilyIndices = nullptr;
    }

    swapchainCreateInfo.preTransform = swapchainDetails.capabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    swapchainCreateInfo.presentMode = presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;

    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    auto result = vkCreateSwapchainKHR(
        this->device,
        &swapchainCreateInfo,
        nullptr,
        &this->swapChain
    );

    if (result != VK_SUCCESS){
        throw std::runtime_error("Failed to create swap chain");
    }

    vkGetSwapchainImagesKHR(this->device, this->swapChain, &imageCount, nullptr);
    this->swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(this->device, this->swapChain, &imageCount, this->swapChainImages.data());

    this->swapChainImageFormat = surfaceFormat.format;
    this->swapChainImageExtent = swapExtent;
}

TriangleApplication::SwapChainSupportDetails TriangleApplication::querySwapChainSupport(const VkPhysicalDevice device){
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, this->surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        device,
        this->surface,
        &formatCount,
        nullptr
    );

    if (formatCount > 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            device,
            this->surface,
            &formatCount,
            details.formats.data()
        );
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device,
        this->surface,
        &presentModeCount,
        nullptr
    );

    if (presentModeCount > 0){
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            device,
            this->surface,
            &presentModeCount,
            details.presentModes.data()
        );
    }

    return details;
}

VkSurfaceFormatKHR TriangleApplication::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats){
    for (const auto& availableFormat : availableFormats){
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
                return availableFormat;
            }
    }

    return availableFormats[0];
}

VkPresentModeKHR TriangleApplication::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes){
    for (const auto& presentMode : availablePresentModes){
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR){
            return presentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D TriangleApplication::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities){
    if (capabilities.currentExtent.width != UINT32_MAX){
        return capabilities.currentExtent;
    }
    else {
        VkExtent2D actualExtent = {0, 0};

        actualExtent.width = std::max(
            capabilities.minImageExtent.width, 
            std::min(
                capabilities.maxImageExtent.width, 
                actualExtent.width
            )
        );

        actualExtent.height = std::max(
            capabilities.minImageExtent.height,
            std::min(
                capabilities.maxImageExtent.height,
                actualExtent.height
            )
        );

        return actualExtent;
    }
}

void TriangleApplication::createImageViews(){
    this->swapChainImageViews.resize(this->swapChainImages.size());

    for (size_t i = 0; i < this->swapChainImages.size(); i++){
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = this->swapChainImages[i];

        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = this->swapChainImageFormat;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        auto result = vkCreateImageView(
            this->device,
            &createInfo,
            nullptr,
            &this->swapChainImageViews[i]
        );

        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create swapchain views");
        }
    }
}

std::vector<char> TriangleApplication::readFile(const std::string& filename){

    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()){
        std::stringstream ss;
        ss << "Failed to open file: " << filename;
        throw std::runtime_error(ss.str());
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();
    return buffer;
}

VkShaderModule TriangleApplication::createShaderModule(const std::vector<char>& code){
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if(vkCreateShaderModule(this->device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS){
        throw std::runtime_error("Failed to create shader module!");
    }

    return shaderModule;
}

void TriangleApplication::createRenderPass(){
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = this->swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    if(vkCreateRenderPass(this->device, &renderPassInfo, nullptr, &this->renderPass) != VK_SUCCESS){
        throw std::runtime_error("Failed to create render pass!");
    }
}

// Set the output path of the shaders here
const char* frag_shader = "shaders/triangle.frag.spv";
const char* vert_shader = "shaders/triangle.vert.spv";

void TriangleApplication::createGraphicsPipeline() {
    auto fragShaderCode = readFile(frag_shader);
    auto vertShaderCode = readFile(vert_shader);

    VkShaderModule vertShaderModule = this->createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = this->createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexStateCreateInfo = {};
    vertexStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexStateCreateInfo.vertexBindingDescriptionCount = 0;
    vertexStateCreateInfo.pVertexBindingDescriptions = nullptr;
    vertexStateCreateInfo.vertexAttributeDescriptionCount = 0;
    vertexStateCreateInfo.pVertexAttributeDescriptions = nullptr;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
    inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = this->swapChainImageExtent.width;
    viewport.height = this->swapChainImageExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = this->swapChainImageExtent;

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
    viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateCreateInfo.viewportCount = 1;
    viewportStateCreateInfo.pViewports = &viewport;
    viewportStateCreateInfo.scissorCount = 1;
    viewportStateCreateInfo.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_LINE_WIDTH
    };

    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if(vkCreatePipelineLayout(this->device, &pipelineLayoutInfo, nullptr, &this->pipelineLayout) != VK_SUCCESS){
        throw std::runtime_error("Failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexStateCreateInfo;
    pipelineInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
    pipelineInfo.pViewportState = &viewportStateCreateInfo;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState; // Divergence from tutorial
    
    pipelineInfo.layout = this->pipelineLayout;
    pipelineInfo.renderPass = this->renderPass;
    pipelineInfo.subpass = 0;

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    if(vkCreateGraphicsPipelines(this->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &this->graphicsPipeline) != VK_SUCCESS){
        throw std::runtime_error("Failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(this->device, fragShaderModule, nullptr);
    vkDestroyShaderModule(this->device, vertShaderModule, nullptr);
}

void TriangleApplication::createFrameBuffers(){
    this->swapChainFramebuffers.resize(this->swapChainImageViews.size());

    for(size_t i = 0; i < this->swapChainImageViews.size(); i++){
        VkImageView attachments[] {
            this->swapChainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = this->renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = this->swapChainImageExtent.width;
        framebufferInfo.height = this->swapChainImageExtent.height;
        framebufferInfo.layers = 1;

        if(vkCreateFramebuffer(this->device, &framebufferInfo, nullptr, &this->swapChainFramebuffers[i]) != VK_SUCCESS){
            throw std::runtime_error("Failed to create framebuffer");
        }
    }
}

void TriangleApplication::createCommandPool(){
    QueueFamilyIndicies queueFamilyIndices = findQueueFamilies(this->physicalDevice);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    poolInfo.flags = 0;

    if(vkCreateCommandPool(this->device, &poolInfo, nullptr, &this->commandPool) != VK_SUCCESS){
        throw std::runtime_error("Failed to create command pool");
    }
}

void TriangleApplication::createCommandBuffers(){
    this->commandBuffers.resize(this->swapChainFramebuffers.size());

    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = this->commandPool;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = (uint32_t) commandBuffers.size();

    if(vkAllocateCommandBuffers(this->device, &allocateInfo, this->commandBuffers.data()) != VK_SUCCESS){
        throw std::runtime_error("Failed to allocate command buffers");
    }

    for(size_t i = 0; i < this->commandBuffers.size(); i++){
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;

        if(vkBeginCommandBuffer(this->commandBuffers[i], &beginInfo) != VK_SUCCESS){
            throw std::runtime_error("Failed to begin recording command buffer");
        }

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = this->renderPass;
        renderPassInfo.framebuffer = this->swapChainFramebuffers[i];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = this->swapChainImageExtent;

        VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(this->commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(this->commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, this->graphicsPipeline);
        vkCmdDraw(this->commandBuffers[i], 3, 1, 0, 0);

        vkCmdEndRenderPass(this->commandBuffers[i]);
        if(vkEndCommandBuffer(this->commandBuffers[i]) != VK_SUCCESS){
            throw std::runtime_error("Failed to record command buffer!");
        }
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