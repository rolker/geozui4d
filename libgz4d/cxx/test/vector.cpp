#include <gz4d/Vector.h>
#include <gz4d/vector/ios.h>

#include <iostream>
#include <sstream>

int main(int argc, char * argv[])
{
    gz4d::Vector<double,3> p;
    p[0] = 1;
    p[2] = 3;
    gz4d::Vector<double,3> p2(p);
    std::cerr << p << std::endl;
    std::cerr << p2 << std::endl;
    std::stringstream ss("2 4 6");
    ss >> p2;
    std::cerr << p2 << std::endl;
    gz4d::Vector<double,2> d2;
    std::cerr << d2 << std::endl;
    std::stringstream ss2("10 11 12 13");
    ss2 >> d2;
    std::cerr << d2 << std::endl;
    gz4d::Vector<double,2> d2_2(3.0);
    std::cerr << d2_2 << std::endl;
    ss2 >> d2_2;
    std::cerr << d2_2 << std::endl;
    return 0;
}
