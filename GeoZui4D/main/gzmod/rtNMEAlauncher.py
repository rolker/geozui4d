#!/usr/bin/env python
# gz
# real-time NMEA
# Manage real time gps and nmea streams

import gz4d.rtNMEA.rtNMEA

if __name__ == '__main__':
	app = gz4d.rtNMEA.rtNMEA.rtNMEAApp()
	app.MainLoop()
