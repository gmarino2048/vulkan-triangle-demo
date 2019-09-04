#include <iostream>
#include <triangle.hpp>

#include <vulkan/vulkan.h>

int main(int, char**) {
    println("Hello World!");
    println("My name is Guy!");
}

void println(std::string str) {
    std::cout << str << "\n";
}