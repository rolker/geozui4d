# GeoZui4D Roadmap

## Goal

Make GeoZui4D usable on modern systems, with the file formats people use
today. The code dates from 2000. It now builds on current Linux with C++17
(ADR-0001), uses std::filesystem (ADR-0002) and reads rasters through GDAL.
The open issues set the course: ROS 2 as a live and recorded data source
(#1), Qt 6 with embedded Python in place of Tcl/Tk (#2), modern OpenGL and
smart pointers in libgz4d and the object classes (#3), and BAG bathymetry
from bagViewer (#6). Each becomes a Direction line when it is taken up.

## Direction

What must be true for the project's current acceptance case, each line with
the check that says whether it is true today. The
[agent workspace](https://github.com/rolker/agent_workspace), which GeoZui4D
can be used with but does not depend on, reads this section and runs every
check in its sit-rep; its `sitrep_checks.sh` holds the check grammar. This
section holds no status and no dates.

**Acceptance case.** GeoZui4D is a test project for the agent workspace and
stays usable with or without it: a fresh clone builds and tests by hand from
the README, the same build and tests run through the workspace, and an
issue here runs through the workspace loop to merge.

| Line | Check |
|------|-------|
| A fresh clone configures, builds and runs its tests by hand (#9) | `gh-issue 9 closed` |
| The README's build steps need nothing from the workspace | `file-contains README.md cmake ..` |
| The workspace builds through the project's own tracked script | `file-exists scripts/build.sh` |
| The checkout is registered on this machine and validates | `script adapter --project gz4d validate` |
| Agents read the project's conventions from one guide that stands on its own | `file-contains .agents/README.md this guide stands on its own` |
| The skills that duplicate workspace skills are gone and the gz4d rules live in `AGENTS.md` (#7) | `gh-issue 7 closed` |
| An issue here has run through the workspace loop to merge | |

| Deferral | Revisit check |
|----------|---------------|
| Direction lines for the long-term goal (#1, #2, #3, #6): after a fresh clone builds, since every modernization change needs that first | `gh-issue 9 closed` |
| Where work plans and progress timelines live (today: committed on each issue's feature branch, the workspace's provisional default); revisit once the workspace's records lookup (rolker/agent_workspace#393) lands | `gh-issue rolker/agent_workspace#393 closed` |
