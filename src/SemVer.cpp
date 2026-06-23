#include "mango/SemVer.hpp"

#include <array>
#include <charconv>
#include <vector>

namespace mango {

namespace {
std::optional<std::uint32_t> parseNumber(std::string_view s) {
    if (s.empty()) {
        return std::nullopt;
    }
    std::uint32_t value = 0;
    const char* begin = s.data();
    const char* end = s.data() + s.size();
    const auto [ptr, ec] = std::from_chars(begin, end, value);
    if (ec != std::errc{} || ptr != end) {
        return std::nullopt;
    }
    return value;
}

std::vector<std::string_view> split(std::string_view s, char delim) {
    std::vector<std::string_view> parts;
    std::size_t start = 0;
    for (std::size_t i = 0; i <= s.size(); ++i) {
        if (i == s.size() || s[i] == delim) {
            parts.push_back(s.substr(start, i - start));
            start = i + 1;
        }
    }
    return parts;
}

std::string_view trim(std::string_view s) {
    while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) {
        s.remove_prefix(1);
    }
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t')) {
        s.remove_suffix(1);
    }
    return s;
}
} // namespace

std::optional<SemVer> SemVer::parse(std::string_view text) {
    text = trim(text);
    if (!text.empty() && (text.front() == 'v' || text.front() == 'V')) {
        text.remove_prefix(1);
    }
    std::string pre;
    if (const auto dash = text.find('-'); dash != std::string_view::npos) {
        pre = std::string(text.substr(dash + 1));
        text = text.substr(0, dash);
    }
    const auto parts = split(text, '.');
    if (parts.empty() || parts.size() > 3) {
        return std::nullopt;
    }
    SemVer v;
    const auto major = parseNumber(parts[0]);
    if (!major) {
        return std::nullopt;
    }
    v.major = *major;
    if (parts.size() >= 2) {
        const auto minor = parseNumber(parts[1]);
        if (!minor) return std::nullopt;
        v.minor = *minor;
    }
    if (parts.size() == 3) {
        const auto patch = parseNumber(parts[2]);
        if (!patch) return std::nullopt;
        v.patch = *patch;
    }
    v.prerelease = std::move(pre);
    return v;
}

std::string SemVer::toString() const {
    std::string s = std::to_string(major) + "." + std::to_string(minor) + "." +
                    std::to_string(patch);
    if (!prerelease.empty()) {
        s += "-" + prerelease;
    }
    return s;
}

std::strong_ordering SemVer::operator<=>(const SemVer& other) const {
    if (auto c = major <=> other.major; c != 0) return c;
    if (auto c = minor <=> other.minor; c != 0) return c;
    if (auto c = patch <=> other.patch; c != 0) return c;
    // A pre-release version is lower than the corresponding release.
    if (prerelease.empty() != other.prerelease.empty()) {
        return prerelease.empty() ? std::strong_ordering::greater
                                  : std::strong_ordering::less;
    }
    return prerelease <=> other.prerelease;
}

bool SemVer::operator==(const SemVer& other) const {
    return (*this <=> other) == std::strong_ordering::equal;
}

std::optional<VersionConstraint> VersionConstraint::parse(std::string_view text) {
    text = trim(text);
    VersionConstraint c;
    c.text_ = std::string(text);

    if (text == "*" || text.empty()) {
        c.kind_ = Kind::Any;
        return c;
    }

    // Range form: ">=A <B"
    if (text.find(' ') != std::string_view::npos && text.find(">=") != std::string_view::npos) {
        const auto parts = split(text, ' ');
        for (auto raw : parts) {
            auto p = trim(raw);
            if (p.starts_with(">=")) {
                auto v = SemVer::parse(p.substr(2));
                if (!v) return std::nullopt;
                c.lower_ = *v;
            } else if (p.starts_with("<")) {
                auto v = SemVer::parse(p.substr(1));
                if (!v) return std::nullopt;
                c.upper_ = *v;
                c.hasUpper_ = true;
            }
        }
        c.kind_ = Kind::Range;
        return c;
    }

    if (text.front() == '^' || text.front() == '~') {
        const bool caret = text.front() == '^';
        auto v = SemVer::parse(text.substr(1));
        if (!v) return std::nullopt;
        c.kind_ = caret ? Kind::Caret : Kind::Tilde;
        c.lower_ = *v;
        c.upper_ = *v;
        c.hasUpper_ = true;
        if (caret) {
            // ^1.2.3 -> >=1.2.3 <2.0.0 (or <0.(x+1).0 when major is 0)
            if (v->major > 0) {
                c.upper_ = SemVer{v->major + 1, 0, 0, {}};
            } else if (v->minor > 0) {
                c.upper_ = SemVer{0, v->minor + 1, 0, {}};
            } else {
                c.upper_ = SemVer{0, 0, v->patch + 1, {}};
            }
        } else {
            // ~1.2.3 -> >=1.2.3 <1.3.0
            c.upper_ = SemVer{v->major, v->minor + 1, 0, {}};
        }
        return c;
    }

    auto v = SemVer::parse(text);
    if (!v) {
        return std::nullopt;
    }
    c.kind_ = Kind::Exact;
    c.lower_ = *v;
    return c;
}

bool VersionConstraint::matches(const SemVer& version) const {
    switch (kind_) {
    case Kind::Any:
        return true;
    case Kind::Exact:
        return version == lower_;
    case Kind::Caret:
    case Kind::Tilde:
    case Kind::Range:
        if (version < lower_) {
            return false;
        }
        if (hasUpper_ && !(version < upper_)) {
            return false;
        }
        return true;
    }
    return false;
}

} // namespace mango
