#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0

# gz
# real-time NMEA
# Manage real time gps and nmea streams

import gz4d.rtNMEA.rtNMEA

if __name__ == '__main__':
	app = gz4d.rtNMEA.rtNMEA.rtNMEAApp()
	app.MainLoop()
