#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <array>
#include <string>
#include <vector>
#include <optional>

namespace triangle {
    class TriangleApplication {
        public:
            struct Vertex{
                glm::vec2 pos;
                glm::vec3 color;

                static VkVertexInputBindingDescription getBindingDescription();
                static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
            };

            const std::vector<Vertex> vertices = {
                {{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
                {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
                {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
            };

        private:
            int maxFramesInFlight;
            std::string title;
            int initialWindowWidth;
            int initialWindowHeight;

            GLFWwindow* window;
            VkInstance vkInstance;
            VkDevice device;
            VkPhysicalDevice physicalDevice;

            std::vector<VkSemaphore> imageAvailableSemaphores;
            std::vector<VkSemaphore> renderFinishedSemaphores;
            std::vector<VkFence> inFlightFences;
            std::vector<VkFence> imagesInFlight;
            size_t currentFrame;

            bool framebufferResized = false;

            std::vector<const char*> deviceExtensions;

            VkSurfaceKHR surface;
            VkQueue graphicsQueue;
            VkQueue presentQueue;

            VkSwapchainKHR swapChain;
            std::vector<VkImage> swapChainImages;
            VkFormat swapChainImageFormat;
            VkExtent2D swapChainImageExtent;

            std::vector<VkImageView> swapChainImageViews;

            VkRenderPass renderPass;
            VkPipelineLayout pipelineLayout;
            VkPipeline graphicsPipeline;

            std::vector<VkFramebuffer> swapChainFramebuffers;

            VkCommandPool commandPool;
            std::vector<VkCommandBuffer> commandBuffers;

            VkBuffer vertexBuffer;
            VkDeviceMemory vertexBufferMemory;

            bool validationLayersEnabled;
            std::vector<const char*> validationLayers;

            VkDebugUtilsMessengerEXT debugMessenger;

        public:
            TriangleApplication(
                std::string title = "Triangle Application",
                int initialWidth = 800,
                int initialHeight = 600,
                int framesInFlight = 2
            );

            void run();

        private:
            void initVulkan();
            void initWindow();
            void mainLoop();
            void cleanUp();

            void drawFrame();

            static void framebufferResizedCallback(GLFWwindow* window, int width, int height);

            void createVkInstance();
            bool checkValidationLayerSupport();
            std::vector<const char*> getRequiredExtensions();

            void createSurface();

            void pickPhysicalDevice();
            bool checkDeviceExtensionSupport(const VkPhysicalDevice device);
            unsigned int rateDeviceSuitability(const VkPhysicalDevice device);

            struct QueueFamilyIndicies {
                std::optional<uint32_t> graphicsFamily;
                std::optional<uint32_t> presentFamily;

                bool isComplete();
            };
            QueueFamilyIndicies findQueueFamilies(VkPhysicalDevice device);

            void createLogicalDevice();

            void createSwapChain();
            void recreateSwapChain();
            void cleanUpSwapChain();
            struct SwapChainSupportDetails{
                VkSurfaceCapabilitiesKHR capabilities;
                std::vector<VkSurfaceFormatKHR> formats;
                std::vector<VkPresentModeKHR> presentModes;
            };
            SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice device);
            VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
            VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
            VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

            void createImageViews();

            void createRenderPass();

            std::vector<char> readFile(const std::string& filename);
            VkShaderModule createShaderModule(const std::vector<char>& code);
            void createGraphicsPipeline();

            void createFrameBuffers();

            void createCommandPool();
            void createCommandBuffers();

            void createSyncObjects();

            void setupDebugMessenger();
            void populateDebugMessengerCreateInfo(
                VkDebugUtilsMessengerCreateInfoEXT& createInfo
            );

            void createVertexBuffers();
            uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

            VkResult createVkDebugMessenger(
                const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkDebugUtilsMessengerEXT* pDebugMessenger
            );
            void destroyVkDebugMessenger(
                const VkAllocationCallbacks* pAllocator = nullptr
            );
            

            static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
                VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageType,
                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                void* pUserData
            );
    };
}