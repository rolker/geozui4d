// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/sonar/DeltaT.h>
#include <gz4d/time.h>
#include <iostream>

int main()
{
    gz4d::DeltaT dt;
    dt.Connect(gz4d::IPSocket::Address("192.168.1.35",4040));
    bool flag = true;
    while(true)
    {
        std::vector<unsigned char> ping =  dt.Ping();
        for (int i = 0; i < 50; ++i)
        {
            std::cerr << i << "\t";
            for(int j = 0; j < 16; ++j)
            {
                //std::cerr << int(ping[i*16+j])-127 << "\t";
                for(int k = 7; k >= 0; --k)
                {
                    if (ping[i*16+j]&(0x01<<k))
                        std::cerr << "*";
                    else
                        std::cerr << "_";
                }
                if (j < 15)
                    std::cerr << ", ";
                else
                    std::cerr << std::endl;
            }
        }
        flag = !flag;
        if(flag)
            std::cerr << "O" << std::endl;
        else
            std::cerr << "o" << std::endl;
        gz4d::Sleep(gz4d::time::Duration(0.2));
    }
}
