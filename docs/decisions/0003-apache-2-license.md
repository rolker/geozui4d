# ADR-0003: Switch to Apache 2.0 License

## Status

Accepted

## Context

GeoZui4D was originally distributed under a research-only license that
restricted use to research purposes. This limited community adoption,
third-party contributions, and integration with other open-source projects.
As part of the open-source modernization effort, a permissive license was
needed to enable broader use.

## Decision

Switch from the research-only license to the Apache License, Version 2.0.

- Replace the LICENSE file with the Apache 2.0 license text
- Copyright holder: Center for Coastal and Ocean Mapping, University of
  New Hampshire (2000-2026)
- The old research-only license file at `GeoZui4D/main/license.txt` is
  superseded by the top-level LICENSE file

Note: Some files in `GeoZui4D/main/widgets/` contain copyright notices from
Interactive Visualization Systems Inc. (IVS). These files predate the project
and retain their original copyright headers.

## Consequences

**Positive:**
- Enables unrestricted use, modification, and distribution
- Allows commercial use and integration with other projects
- Patent grant provides additional legal clarity for users
- Standard license recognized by OSI, compatible with most open-source licenses

**Negative:**
- Cannot restrict use to research-only contexts anymore
- Contributors must agree to Apache 2.0 terms
