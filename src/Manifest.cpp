#include "mango/Manifest.hpp"

#include "mango/Json.hpp"

#include <fstream>
#include <sstream>

namespace mango {

namespace {
std::string getString(const Json& obj, std::string_view key, std::string fallback = {}) {
    const Json* v = obj.find(key);
    return (v && v->isString()) ? v->asString() : std::move(fallback);
}

void readDeps(const Json& obj, std::string_view key, std::vector<Dependency>& out) {
    const Json* deps = obj.find(key);
    if (!deps || !deps->isObject()) {
        return;
    }
    for (const auto& [name, spec] : deps->object()) {
        Dependency dep;
        dep.name = name;
        if (spec.isString()) {
            dep.constraint = spec.asString();
        } else if (spec.isObject()) {
            // { "version": "^1.0.0" } | { "path": "..." } | { "git": "..." }
            if (const Json* v = spec.find("version"); v && v->isString()) {
                dep.constraint = v->asString();
            } else {
                dep.constraint = "*";
            }
        }
        out.push_back(std::move(dep));
    }
}
} // namespace

std::optional<Manifest> Manifest::parse(std::string_view text, std::string& error) {
    auto json = Json::parse(text, error);
    if (!json) {
        return std::nullopt;
    }
    if (!json->isObject()) {
        error = "manifest: top-level value must be an object";
        return std::nullopt;
    }

    Manifest m;
    m.name = getString(*json, "name");
    if (m.name.empty()) {
        error = "manifest: missing required field 'name'";
        return std::nullopt;
    }
    const std::string versionText = getString(*json, "version", "0.0.0");
    if (auto v = SemVer::parse(versionText)) {
        m.version = *v;
    } else {
        error = "manifest: invalid 'version': " + versionText;
        return std::nullopt;
    }
    m.edition = getString(*json, "edition", "2026");
    m.description = getString(*json, "description");
    m.entry = getString(*json, "entry", "src/main.m");
    readDeps(*json, "dependencies", m.dependencies);
    readDeps(*json, "devDependencies", m.devDependencies);
    return m;
}

std::optional<Manifest> Manifest::load(const std::string& path, std::string& error) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        error = "manifest: cannot open '" + path + "'";
        return std::nullopt;
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    return parse(ss.str(), error);
}

std::string Manifest::toJson() const {
    std::string s = "{\n";
    s += "  \"name\": \"" + name + "\",\n";
    s += "  \"version\": \"" + version.toString() + "\",\n";
    s += "  \"edition\": \"" + edition + "\",\n";
    if (!description.empty()) {
        s += "  \"description\": \"" + description + "\",\n";
    }
    s += "  \"entry\": \"" + entry + "\",\n";
    s += "  \"dependencies\": {";
    for (std::size_t i = 0; i < dependencies.size(); ++i) {
        s += i == 0 ? "\n" : ",\n";
        s += "    \"" + dependencies[i].name + "\": \"" + dependencies[i].constraint + "\"";
    }
    s += dependencies.empty() ? "}\n" : "\n  }\n";
    s += "}\n";
    return s;
}

} // namespace mango
