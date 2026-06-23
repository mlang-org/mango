// Dependency-free tests for the mango core (SemVer, JSON, Manifest).
#include "mango/Json.hpp"
#include "mango/Manifest.hpp"
#include "mango/SemVer.hpp"

#include <iostream>
#include <string>

namespace {
int g_failures = 0;
std::string g_current;

void check(bool cond, const std::string& msg) {
    if (!cond) {
        ++g_failures;
        std::cerr << "  FAIL [" << g_current << "]: " << msg << "\n";
    }
}

using namespace mango;

void test_semver_parse_and_order() {
    g_current = "semver_parse_and_order";
    auto a = SemVer::parse("1.2.3");
    check(a.has_value(), "parses 1.2.3");
    check(a->major == 1 && a->minor == 2 && a->patch == 3, "fields correct");
    check(SemVer::parse("v2.0").has_value(), "parses v-prefix and short form");
    check(!SemVer::parse("abc").has_value(), "rejects garbage");

    auto lo = *SemVer::parse("1.0.0");
    auto hi = *SemVer::parse("1.2.0");
    check(lo < hi, "1.0.0 < 1.2.0");
    check(*SemVer::parse("1.0.0-alpha") < *SemVer::parse("1.0.0"), "prerelease < release");
}

void test_caret_constraint() {
    g_current = "caret_constraint";
    auto c = VersionConstraint::parse("^1.4.0");
    check(c.has_value(), "parses ^1.4.0");
    check(c->matches(*SemVer::parse("1.4.0")), "1.4.0 matches ^1.4.0");
    check(c->matches(*SemVer::parse("1.9.9")), "1.9.9 matches ^1.4.0");
    check(!c->matches(*SemVer::parse("2.0.0")), "2.0.0 does not match ^1.4.0");
    check(!c->matches(*SemVer::parse("1.3.0")), "1.3.0 does not match ^1.4.0");
}

void test_tilde_and_range() {
    g_current = "tilde_and_range";
    auto t = VersionConstraint::parse("~1.2.3");
    check(t->matches(*SemVer::parse("1.2.9")), "1.2.9 matches ~1.2.3");
    check(!t->matches(*SemVer::parse("1.3.0")), "1.3.0 does not match ~1.2.3");

    auto r = VersionConstraint::parse(">=2.0.0 <3.0.0");
    check(r.has_value(), "parses range");
    check(r->matches(*SemVer::parse("2.5.0")), "2.5.0 in range");
    check(!r->matches(*SemVer::parse("3.0.0")), "3.0.0 excluded");
    check(!r->matches(*SemVer::parse("1.9.0")), "1.9.0 below range");
}

void test_json_parse() {
    g_current = "json_parse";
    std::string err;
    auto j = Json::parse(R"({"a": 1, "b": ["x", true], "c": {"d": "e"}})", err);
    check(j.has_value(), "parses object: " + err);
    check(j->isObject(), "is object");
    check(j->find("a") != nullptr, "has key a");
    check(j->find("b") && j->find("b")->isArray(), "b is array");
    check(j->find("c") && j->find("c")->find("d") != nullptr, "nested object");
}

void test_manifest_parse() {
    g_current = "manifest_parse";
    std::string err;
    auto m = Manifest::parse(R"({
        "name": "com.example.app",
        "version": "1.2.0",
        "dependencies": { "std.http": "^1.4.0", "com.acme.json": ">=2.0.0 <3.0.0" }
    })", err);
    check(m.has_value(), "parses manifest: " + err);
    check(m->name == "com.example.app", "name");
    check(m->version.minor == 2, "version");
    check(m->dependencies.size() == 2, "two dependencies");
}

void test_manifest_roundtrip() {
    g_current = "manifest_roundtrip";
    std::string err;
    Manifest m;
    m.name = "x.y.z";
    m.version = *SemVer::parse("0.3.1");
    m.dependencies.push_back({"a.b", "^1.0.0"});
    auto reparsed = Manifest::parse(m.toJson(), err);
    check(reparsed.has_value(), "round-trips: " + err);
    check(reparsed && reparsed->name == "x.y.z", "name survives");
    check(reparsed && reparsed->dependencies.size() == 1, "dependency survives");
}

} // namespace

int main() {
    test_semver_parse_and_order();
    test_caret_constraint();
    test_tilde_and_range();
    test_json_parse();
    test_manifest_parse();
    test_manifest_roundtrip();

    if (g_failures == 0) {
        std::cout << "All mango core tests passed.\n";
        return 0;
    }
    std::cerr << g_failures << " check(s) failed.\n";
    return 1;
}
