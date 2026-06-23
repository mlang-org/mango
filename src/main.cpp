// mango - the MLang package manager CLI. Manifest parsing, semantic-version
// resolution, lockfiles, fetching, and publishing (docs/resolution.md and the
// MLang design chapter 21). This driver implements the local manifest commands;
// network operations are scaffolded.
#include "mango/Manifest.hpp"
#include "mango/SemVer.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#ifndef MANGO_VERSION
#define MANGO_VERSION "0.1.0"
#endif

namespace {

constexpr const char* kManifestFile = "mango.json";

void usage(std::ostream& os) {
    os << "mango " << MANGO_VERSION << " - the MLang package manager\n\n"
       << "USAGE:\n"
       << "    mango <command> [args]\n\n"
       << "COMMANDS:\n"
       << "    init               create a mango.json in the current directory\n"
       << "    add <pkg> [ver]    add a dependency\n"
       << "    remove <pkg>       remove a dependency\n"
       << "    tree               print the dependency graph\n"
       << "    install            resolve, fetch, and lock dependencies\n"
       << "    update [pkg]       re-resolve within constraints\n"
       << "    search <query>     search the registry\n"
       << "    publish            publish the package to the registry\n"
       << "    --version          print version\n"
       << "    --help             print this help\n";
}

bool writeFile(const std::string& path, const std::string& contents) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) {
        std::cerr << "mango: cannot write '" << path << "'\n";
        return false;
    }
    out << contents;
    return true;
}

int cmdInit() {
    std::ifstream existing(kManifestFile);
    if (existing) {
        std::cerr << "mango: " << kManifestFile << " already exists\n";
        return 1;
    }
    mango::Manifest m;
    m.name = "com.example.app";
    m.version = mango::SemVer{0, 1, 0, {}};
    m.edition = "2026";
    m.entry = "src/main.m";
    if (!writeFile(kManifestFile, m.toJson())) {
        return 1;
    }
    std::cout << "Created " << kManifestFile << "\n";
    return 0;
}

int cmdTree() {
    std::string error;
    auto m = mango::Manifest::load(kManifestFile, error);
    if (!m) {
        std::cerr << "mango: " << error << "\n";
        return 1;
    }
    std::cout << m->name << "@" << m->version.toString() << "\n";
    if (m->dependencies.empty()) {
        std::cout << "  (no dependencies)\n";
    }
    for (std::size_t i = 0; i < m->dependencies.size(); ++i) {
        const bool last = i + 1 == m->dependencies.size();
        const auto& dep = m->dependencies[i];
        std::cout << (last ? "  \\-- " : "  |-- ") << dep.name << " " << dep.constraint << "\n";
    }
    return 0;
}

int cmdAdd(const std::string& pkg, const std::string& constraint) {
    std::string error;
    auto m = mango::Manifest::load(kManifestFile, error);
    if (!m) {
        std::cerr << "mango: " << error << "\n";
        return 1;
    }
    const std::string spec = constraint.empty() ? "*" : constraint;
    if (auto c = mango::VersionConstraint::parse(spec); !c) {
        std::cerr << "mango: invalid version constraint '" << spec << "'\n";
        return 1;
    }
    for (auto& dep : m->dependencies) {
        if (dep.name == pkg) {
            dep.constraint = spec;
            writeFile(kManifestFile, m->toJson());
            std::cout << "Updated " << pkg << " to " << spec << "\n";
            return 0;
        }
    }
    m->dependencies.push_back(mango::Dependency{pkg, spec});
    writeFile(kManifestFile, m->toJson());
    std::cout << "Added " << pkg << " " << spec << "\n";
    return 0;
}

int cmdRemove(const std::string& pkg) {
    std::string error;
    auto m = mango::Manifest::load(kManifestFile, error);
    if (!m) {
        std::cerr << "mango: " << error << "\n";
        return 1;
    }
    const auto before = m->dependencies.size();
    std::erase_if(m->dependencies, [&](const mango::Dependency& d) { return d.name == pkg; });
    if (m->dependencies.size() == before) {
        std::cerr << "mango: '" << pkg << "' is not a dependency\n";
        return 1;
    }
    writeFile(kManifestFile, m->toJson());
    std::cout << "Removed " << pkg << "\n";
    return 0;
}

int cmdNetwork(const std::string& what) {
    std::string error;
    auto m = mango::Manifest::load(kManifestFile, error);
    if (!m) {
        std::cerr << "mango: " << error << "\n";
        return 1;
    }
    std::cerr << "mango: '" << what
              << "' needs the registry client, which is under construction. "
                 "The manifest parsed and validated successfully ("
              << m->dependencies.size() << " dependencies).\n";
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    std::vector<std::string> args(argv + 1, argv + argc);
    if (args.empty() || args[0] == "--help" || args[0] == "help") {
        usage(std::cout);
        return args.empty() ? 2 : 0;
    }
    if (args[0] == "--version") {
        std::cout << "mango " << MANGO_VERSION << "\n";
        return 0;
    }

    const std::string& cmd = args[0];
    if (cmd == "init") {
        return cmdInit();
    }
    if (cmd == "tree") {
        return cmdTree();
    }
    if (cmd == "add") {
        if (args.size() < 2) { std::cerr << "mango: add requires a package name\n"; return 2; }
        return cmdAdd(args[1], args.size() >= 3 ? args[2] : std::string());
    }
    if (cmd == "remove") {
        if (args.size() < 2) { std::cerr << "mango: remove requires a package name\n"; return 2; }
        return cmdRemove(args[1]);
    }
    if (cmd == "install" || cmd == "update" || cmd == "search" || cmd == "publish") {
        return cmdNetwork(cmd);
    }

    std::cerr << "mango: unknown command '" << cmd << "'\n\n";
    usage(std::cerr);
    return 2;
}
