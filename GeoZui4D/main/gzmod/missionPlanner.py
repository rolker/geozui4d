#!/usr/bin/python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0

# gz
# missionPlanner
# Create and edit AUV missions

import gz4d.MissionPlan.missionPlannerApp

if __name__ == '__main__':
	app = gz4d.MissionPlan.missionPlannerApp.missionPlannerApp()
	app.MainLoop()
