#include<gtest/gtest.h>

#include "../include/gz4d/socket/IPSocket.h"

TEST(socket,IPAddressURI)
{
    gz4d::URI u("http://localhost");
    gz4d::IPSocket::Address a(u);
    
    EXPECT_EQ("127.0.0.1",a.GetAddress());
    EXPECT_EQ(80,a.GetPort());
    
}
