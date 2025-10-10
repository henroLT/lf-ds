#include <iostream>
#include "headers/lfqueue.hpp"

int main() {
    lfqueue<int> test;
    test.push(5);
    std::cout << test.pop() << std::endl;
}