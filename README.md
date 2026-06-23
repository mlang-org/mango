<div align="center">

# mango

**The package manager for the [MLang](https://github.com/mlang-org/mlang) programming language.**

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![C++23](https://img.shields.io/badge/written%20in-C%2B%2B23-00599C.svg)](src/)

</div>

---

`mango` manages MLang dependencies: declaring them in a manifest, resolving a
consistent version set with semantic versioning, fetching them, locking them for
reproducible builds, and publishing your own packages to a registry. It is to
MLang what Cargo is to Rust.

It is developed in its own repository and linked into the MLang monorepo as a
git submodule at `packages/mango`. The full design is in
[docs/design/21-mango.md](https://github.com/mlang-org/mlang/blob/main/docs/design/21-mango.md).

## The manifest: `mango.json`

```json
{
  "name": "com.example.app",
  "version": "1.2.0",
  "edition": "2026",
  "entry": "src/main.m",
  "dependencies": {
    "std.http": "^1.4.0",
    "com.acme.json": ">=2.0.0 <3.0.0"
  }
}
```

## Commands

| Command | Effect |
|---------|--------|
| `mango init` | Create a `mango.json` in the current directory. |
| `mango add <pkg>[@ver]` | Add a dependency and update the manifest. |
| `mango remove <pkg>` | Remove a dependency. |
| `mango install` | Resolve, fetch, and lock dependencies. |
| `mango update [pkg]` | Re-resolve within constraints; bump the lock. |
| `mango tree` | Print the resolved dependency graph. |
| `mango search <query>` | Search the registry. |
| `mango publish` | Package and upload to the registry. |

## Building

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build
./build/bin/mango --help
```

Requires a C++23 compiler and CMake >= 3.28.

## Status

Early development. The manifest parser, the semantic-version engine, and the CLI
dispatch are implemented; network fetching, the full resolver, and the registry
client are under construction (see the design document).

## License

MIT. See [LICENSE](LICENSE).
