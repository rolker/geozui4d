#include <gz4d/socket/IPSocket.h>
#include <iostream>

int main(int argc, char * argv[])
{
    std::vector<std::string> a = gz4d::IPSocket::Interfaces();
    for (std::vector<std::string>::iterator i = a.begin(); i!= a.end(); ++i)
        std::cout << *i << std::endl;
}
