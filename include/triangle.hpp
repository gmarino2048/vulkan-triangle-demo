#include<string>

namespace triangle {
    class TriangleApplication {
        private:
            std::string title;
            int initialWindowWidth;
            int initialWindowHeight;

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
    };
}