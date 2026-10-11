#include <Eigen/Dense>
#include <iostream>

int main()
{
    Eigen::Vector3d a(1, 2, 3);
    Eigen::Vector3d b(4, 5, 6);

    std::cout << (a + b).transpose() << '\n';
}