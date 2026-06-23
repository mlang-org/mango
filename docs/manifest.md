# The `mango.json` manifest

Every MLang package is a directory containing a `mango.json` manifest. This
document specifies its fields. The format is JSON (with tolerance for `//`
comments and trailing commas on read).

## Fields

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `name` | string | yes | The package name, in reverse-DNS form (`com.example.app`). |
| `version` | string | yes | The package version (semantic version, e.g. `1.2.0`). |
| `edition` | string | no | The language edition (default `2026`). |
| `description` | string | no | A one-line description. |
| `entry` | string | no | The entry module (default `src/main.m`). |
| `dependencies` | object | no | Runtime dependencies (see below). |
| `devDependencies` | object | no | Dependencies needed only for tests/benches. |

## Dependency specifications

A dependency maps a package name to a constraint. The value can be a string
(a version constraint) or an object for non-registry sources:

```json
{
  "dependencies": {
    "std.http": "^1.4.0",
    "com.acme.json": ">=2.0.0 <3.0.0",
    "com.local.utils": { "path": "../utils" },
    "com.git.lib":     { "git": "https://github.com/x/lib.git", "tag": "v0.9.1" }
  }
}
```

## Version constraints

| Syntax | Meaning |
|--------|---------|
| `1.2.3` | exactly `1.2.3` |
| `^1.2.3` | `>=1.2.3 <2.0.0` (compatible within the major; `<0.(x+1).0` when major is 0) |
| `~1.2.3` | `>=1.2.3 <1.3.0` (compatible within the minor) |
| `>=2.0.0 <3.0.0` | an explicit half-open range |
| `*` | any version |

Constraint semantics are implemented in `SemVer.cpp` and covered by tests.

## Example

```json
{
  "name": "com.example.app",
  "version": "1.2.0",
  "edition": "2026",
  "description": "An example MLang application.",
  "entry": "src/main.m",
  "dependencies": {
    "std.http": "^1.4.0"
  },
  "devDependencies": {
    "com.test.bench": "^0.5.0"
  }
}
```
