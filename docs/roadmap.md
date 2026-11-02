# GeoZui4D Roadmap

## Direction

What must be true for the project's current acceptance case, each line with
the check that says whether it is true today. The sit-rep of the
[agent workspace](https://github.com/rolker/agent_workspace)
(`rolker/agent_workspace`, a separate repository of AI-agent infrastructure
with which a checkout can be registered on one's own machine; using it is
optional) reads this section
(its `sitrep_checks.sh` holds the check grammar) and runs every check each
time; this section holds no status and no dates. Without the workspace, the
table is still a plain statement of what the project is working towards.

**Acceptance case.** GeoZui4D is worked from the agent workspace: the
checkout is registered and validates, builds and tests go through the
adapter, agents read the project's conventions from one guide, and an
issue here runs through the workspace loop to merge.

| Line | Check |
|------|-------|
| The checkout is registered on this machine and validates | `script adapter --project gz4d validate` |
| The build goes through the project's own tracked script | `file-exists scripts/build.sh` |
| A fresh clone or worktree configures and builds, and the gtest suite runs (#9) | `gh-issue 9 closed` |
| Agents read the project's conventions from one guide | `file-exists .agents/README.md` |
| The skills that duplicate workspace skills are gone and the gz4d rules live in `AGENTS.md` (#7) | `gh-issue 7 closed` |

| Deferral | Revisit check |
|----------|---------------|
| Where work plans and progress timelines live (today: committed on each issue's feature branch, the workspace's provisional default); revisit once the workspace's records lookup (rolker/agent_workspace#393) lands | `gh-issue rolker/agent_workspace#393 closed` |
