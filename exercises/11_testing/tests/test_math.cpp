#include "calculator.hpp"
#include <iostream>

int main()
{
    const int actual = add(2, 3);

    if (actual != 5) {
        std::cerr << "Expected 5, got " << actual << '\n';
        return 1;
    }

    std::cout << "Test passed\n";
    return 0;
}