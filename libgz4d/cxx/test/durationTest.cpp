#include <gz4d/time/Duration.h>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <gz4d/time/ios.h>

template <typename T> void testDuration(T s)
{
    std::cout << s << " ";
    gz4d::time::Duration d = gz4d::time::Duration(s);
    std::cout << d << " " << d.asSeconds() << std::endl;
}

int main(int argc, char *argv[])
{
    testDuration("0.0s");
    testDuration("1.5s");
    testDuration("-2.1s");
    testDuration(".5m");
    testDuration("-3.4m");
    testDuration("13.6h");
    testDuration("-4.5h");
    testDuration("3.123d");
    testDuration("-5.5d");
    testDuration("2.5w");
    testDuration("-7w");
    testDuration("123456m");
    testDuration("-987654.123h");
    return 0;
}
