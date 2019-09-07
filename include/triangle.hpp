#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <vector>
#include <optional>

namespace triangle {
    class TriangleApplication {
        private:
            std::string title;
            int initialWindowWidth;
            int initialWindowHeight;

            GLFWwindow* window;
            VkInstance vkInstance;
            VkDevice device;
            VkPhysicalDevice physicalDevice;

            VkSurfaceKHR surface;
            VkQueue graphicsQueue;
            VkQueue presentQueue;

            bool validationLayersEnabled;
            std::vector<const char*> validationLayers;

            VkDebugUtilsMessengerEXT debugMessenger;

        public:
            TriangleApplication(
                std::string title = "Triangle Application",
                int initialWidth = 800,
                int initialHeight = 600
            );

            void run();

        private:
            void initVulkan();
            void initWindow();
            void mainLoop();
            void cleanUp();

            void createVkInstance();
            bool checkValidationLayerSupport();
            std::vector<const char*> getRequiredExtensions();

            void createSurface();

            void pickPhysicalDevice();
            unsigned int rateDeviceSuitability(const VkPhysicalDevice device);

            struct QueueFamilyIndicies {
                std::optional<uint32_t> graphicsFamily;
                std::optional<uint32_t> presentFamily;

                bool isComplete();
            };
            QueueFamilyIndicies findQueueFamilies(VkPhysicalDevice device);

            void createLogicalDevice();

            void setupDebugMessenger();
            void populateDebugMessengerCreateInfo(
                VkDebugUtilsMessengerCreateInfoEXT& createInfo
            );

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