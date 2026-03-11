# Architecture Decision Records

This directory contains Architecture Decision Records (ADRs) for the gz4d
project. ADRs capture significant design decisions along with their context
and consequences.

## Format

Each ADR uses a lightweight MADR-inspired format:

- **Status**: Accepted, Superseded, or Deprecated
- **Context**: The problem or situation that prompted the decision
- **Decision**: What was decided and why
- **Consequences**: Positive and negative outcomes

## Index

| ADR | Title | Status |
|-----|-------|--------|
| [0001](0001-cmake-cpp17.md) | Upgrade to CMake 3.14 and C++17 | Accepted |
| [0002](0002-std-filesystem.md) | Replace boost::filesystem with std::filesystem | Accepted |
| [0003](0003-apache-2-license.md) | Switch to Apache 2.0 License | Accepted |

## Creating a New ADR

1. Copy the next sequential number
2. Use the format: `NNNN-short-title.md`
3. Fill in Status, Context, Decision, Consequences
4. Add an entry to this index
5. ADRs are immutable once accepted -- supersede rather than edit
