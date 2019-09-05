#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <vector>

namespace triangle {
    class TriangleApplication {
        private:
            std::string title;
            int initialWindowWidth;
            int initialWindowHeight;

            GLFWwindow* window;
            VkInstance vkInstance;

            bool validationLayersEnabled;
            std::vector<const char*> validationLayers;

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
    };
}