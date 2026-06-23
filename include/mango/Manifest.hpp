#ifndef MANGO_MANIFEST_HPP
#define MANGO_MANIFEST_HPP

#include "mango/SemVer.hpp"

#include <optional>
#include <string>
#include <vector>

namespace mango {

struct Dependency {
    std::string name;
    std::string constraint; // raw constraint text, e.g. "^1.4.0"
};

// The parsed contents of a mango.json manifest.
struct Manifest {
    std::string name;
    SemVer version;
    std::string edition;
    std::string description;
    std::string entry = "src/main.m";
    std::vector<Dependency> dependencies;
    std::vector<Dependency> devDependencies;

    // Parses manifest text. On failure, returns nullopt and sets `error`.
    static std::optional<Manifest> parse(std::string_view text, std::string& error);

    // Reads and parses a manifest file from disk.
    static std::optional<Manifest> load(const std::string& path, std::string& error);

    // Serializes back to canonical mango.json text.
    std::string toJson() const;
};

} // namespace mango

#endif
