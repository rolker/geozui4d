#!/usr/bin/python
# gz
# missionPlanner
# Create and edit AUV missions

import gz4d.MissionPlan.missionPlannerApp

if __name__ == '__main__':
	app = gz4d.MissionPlan.missionPlannerApp.missionPlannerApp()
	app.MainLoop()
