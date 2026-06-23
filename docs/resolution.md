# Dependency resolution

`mango` turns the constraints in a `mango.json` (and those of its transitive
dependencies) into a single, consistent set of exact versions, recorded in a
`mango.lock` for reproducible builds. This document describes the algorithm.

## Goals

1. **Consistency.** Pick one version per package per major version where
   possible; surface a clear conflict when no consistent set exists.
2. **Newest-compatible.** Prefer the newest version that satisfies all
   constraints.
3. **Reproducibility.** Write a lockfile pinning exact versions plus content
   hashes, so a build is identical across machines and time.
4. **Determinism.** The same inputs always produce the same lock.

## Algorithm

`mango` uses a PubGrub-style constraint solver:

1. **Build the constraint graph.** Start from the root manifest's dependencies;
   for each candidate, fetch its manifest and add its dependencies.
2. **Unit propagation.** Maintain a partial solution. When a package's allowed
   version set is narrowed to a single choice, assign it and propagate the
   consequences to dependents.
3. **Decision making.** When propagation stalls, pick an unassigned package and
   try its newest compatible version.
4. **Conflict-driven backtracking.** On a contradiction, derive the incompatible
   set of decisions, record it as a new incompatibility (so the same dead end is
   never revisited), and backjump.
5. **Termination.** The process ends with either a complete assignment (the
   resolution) or a minimal explanation of why no resolution exists.

This is the same class of algorithm used by Cargo and Dart's pub. It gives
clear, actionable conflict messages (`package A needs json ^1, package B needs
json ^2`) instead of an arbitrary failure.

## Multiple majors

Two different *major* versions of a package are treated as distinct packages
(the major is part of the resolved module path), so they may coexist. This
avoids diamond conflicts at the cost of compiling both, which is the right trade
when two dependencies genuinely need incompatible majors.

## The lockfile

`mango.lock` records, for every package in the resolution: the exact version,
the source (registry/git/path), and a content hash. Subsequent builds use the
lock directly and verify hashes on fetch; a mismatch is a hard error
(supply-chain integrity).

## Current status

The `SemVer` constraint engine (this repository) is implemented and tested. The
full solver, the registry client, and lockfile writing are under construction;
see the project README.
