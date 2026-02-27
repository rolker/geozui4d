#include <gz4d/ptu/Ptu.h>
#include <iostream>

int main(int argc, char * argv[])
{
    gz4d::Ptu ptu(argv[1]);
    std::cout << "Pan range: " << ptu.GetPanMin() << " - " << ptu.GetPanMax() << std::endl;
    std::cout << "Tilt range: " << ptu.GetTiltMin() << " - " << ptu.GetTiltMax() << std::endl;
    std::cout << "Pan speed range: " << ptu.GetPanMinSpeed() << " - " << ptu.GetPanMaxSpeed() << std::endl;
    std::cout << "Tilt speed range: " << ptu.GetTiltMinSpeed() << " - " << ptu.GetTiltMaxSpeed() << std::endl;

    return 0;
}