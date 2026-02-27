#!/usr/bin/env python

import sys
import gz4d

use_filter = False
show_raw = False

if len(sys.argv) == 1:
    show_raw = True

for arg in sys.argv[1:]:
    if arg == '-f':
        use_filter = True
    if arg == '-r':
        show_raw = True

f = gz4d.IMU.Filter()

imu = gz4d.IMU.BTSource()
q = imu.NewOutQueue()
imu.Open("00:06:66:00:D9:EE")
while True:
    s = q.pop()
    if show_raw:
        print ', '.join((str(float(s.time)),str(s.count),str(s.mag[0]),str(s.mag[1]),str(s.mag[2]),str(s.accel[0]),str(s.accel[1]),str(s.accel[2]),str(s.gyro[0]),str(s.gyro[1]),str(s.gyro[2])))
    if use_filter:
        fs = f(s)
        print ', '.join((str(float(fs.time)),'filtered',str(fs.mag[0]),str(fs.mag[1]),str(fs.mag[2]),str(fs.accel[0]),str(fs.accel[1]),str(fs.accel[2]),str(fs.gyro[0]),str(fs.gyro[1]),str(fs.gyro[2])))
