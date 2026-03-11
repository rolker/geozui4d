---
name: audit-project
description: Audit the project for license headers, build health, documentation coverage, ADR currency, and third-party code boundaries. Reports findings without making changes.
---

# Audit Project

## Usage

```
/audit-project
```

## Overview

Check the gz4d project against its own conventions and standards. Reports
what's present, what's missing, and what may have drifted. Useful for
periodic health checks or before a release.

**Report, don't fix** -- identify gaps without making changes. Fixes should
be separate tasks.

## Steps

### 1. Check license headers

Scan source files (`.cpp`, `.h`, `.tcl`) for license or copyright headers.
Report:
- Files with Apache 2.0 headers
- Files with other license headers (IVS, etc.)
- Files with no license header

Do not flag vendored third-party code (Togl, BWidget, RapidXML).

### 2. Check build succeeds

```bash
mkdir -p build && cd build && cmake .. && make -j$(nproc)
```

Report build status: success, warnings (count), or errors.

### 3. Check documentation coverage

| Item | Status |
|------|--------|
| `README.md` | Present / Missing / Outdated |
| `AGENTS.md` | Present / Missing |
| `CLAUDE.md` | Present / Missing |
| `docs/decisions/README.md` | Present / Missing |
| ADR index matches files | Yes / No |

Check that README sections are accurate:
- Build instructions work
- Project structure matches actual layout
- License section matches LICENSE file

### 4. Check ADR currency

For each ADR in `docs/decisions/`:
- Is the decision still reflected in the codebase?
- Are there contradictions? (e.g., ADR says use std::filesystem but
  boost::filesystem appears in code)

### 5. Check third-party code boundaries

Verify that vendored code has not been modified:
- `GeoZui4D/main/Togl/`
- `GeoZui4D/main/Togl2.0/`
- `GeoZui4D/main/BWidget-1.7.0/`
- `libgz4d/cxx/include/gz4d/util/rapidxml/`

Check using `git log --oneline -5 -- <path>` to see if recent commits
touched these directories.

### 6. Check IVS copyright files

Verify that IVS copyright files in `GeoZui4D/main/widgets/` have not been
modified recently: Sv_Matrix4.h, Sv_Matrix4.cpp, Sv_Quaternion.h,
Sv_Quaternion.cpp, Sv_Raster.h.

### 7. Produce the report

```markdown
## Project Audit: gz4d

### License Headers

| Category | Count | Examples |
|----------|-------|---------|
| Apache 2.0 | N | ... |
| IVS copyright | N | ... |
| No header | N | ... |

### Build Status

<pass/fail, warning count>

### Documentation Coverage

| Item | Status |
|------|--------|
| ... | ... |

### ADR Currency

| ADR | Current | Notes |
|-----|---------|-------|
| ... | ... | ... |

### Third-Party Boundaries

| Directory | Respected | Last touched |
|-----------|-----------|-------------|
| ... | ... | ... |

### Recommended Actions

- [ ] <specific action items>
```

## Guidelines

- **Report, don't fix** -- identify gaps, don't fill them.
- **Coverage, not quality** -- check what exists, not whether it's good.
- **Be specific** -- list actual file paths and counts, not vague assessments.
