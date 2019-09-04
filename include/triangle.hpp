#include<string>

namespace triangle {
    class TriangleApplication {
        public:
            void run();

        private:
            void initVulkan();
            void initWindow();
            void mainLoop();
            void cleanUp();
    };
}