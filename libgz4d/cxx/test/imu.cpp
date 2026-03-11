// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/IMU/BTSource.h>
#include <gz4d/IMU/Filter.h>
#include <gz4d/time.h>

#include <iostream>

int main(int argc, char **argv)
{
    bool use_filter = false;
    bool show_raw = false;

    std::vector<std::string> args(argv, argv + argc);
    if (args.size() == 1)
        show_raw = true;

    for (std::vector<std::string>::iterator a = args.begin(); a != args.end(); ++a)
    {
        if (*a == "-f")
            use_filter = true;
        if (*a == "-r")
            show_raw = true;
    }

    gz4d::IMU::Filter filter;

    gz4d::IMU::BTSource imu;
    std::shared_ptr<gz4d::ThreadedQueue<gz4d::IMU::RawState> > q = imu.NewOutQueue();
    imu.Open("00:06:66:00:D9:EE");
    while (true)
    {
        gz4d::IMU::RawState s = q->pop();
        if (show_raw)
        {
            std::cout << std::fixed << s.time.secondsSinceEpoch().asSeconds() << "," << s.count << "," << s.mag[0] << "," << s.mag[1] << "," << s.mag[2] << "," << s.accel[0] << ","  << s.accel[1] << "," << s.accel[2] << "," << s.gyro[0] << "," << s.gyro[1] << "," << s.gyro[2] << std::endl;
        }
        if (use_filter)
        {
            gz4d::IMU::State fs = filter(s);
            std::cout << std::fixed << fs.time.secondsSinceEpoch().asSeconds() << "," << "filtered" << "," << fs.mag[0] << "," << fs.mag[1] << "," << fs.mag[2] << "," << fs.accel[0] << ","  << fs.accel[1] << "," << fs.accel[2] << "," << fs.gyro[0] << "," << fs.gyro[1] << "," << fs.gyro[2] << std::endl;
        }

    }
}
