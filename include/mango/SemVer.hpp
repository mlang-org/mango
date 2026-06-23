#ifndef MANGO_SEMVER_HPP
#define MANGO_SEMVER_HPP

#include <compare>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace mango {

// A semantic version (major.minor.patch with an optional pre-release tag).
// See https://semver.org and docs/resolution.md.
struct SemVer {
    std::uint32_t major = 0;
    std::uint32_t minor = 0;
    std::uint32_t patch = 0;
    std::string prerelease; // empty for a normal release

    static std::optional<SemVer> parse(std::string_view text);
    std::string toString() const;

    std::strong_ordering operator<=>(const SemVer& other) const;
    bool operator==(const SemVer& other) const;
};

// A version constraint such as "^1.4.0", ">=2.0.0 <3.0.0", "1.2.3", or "*".
class VersionConstraint {
public:
    static std::optional<VersionConstraint> parse(std::string_view text);

    bool matches(const SemVer& version) const;
    const std::string& text() const { return text_; }

private:
    enum class Kind { Any, Exact, Caret, Tilde, Range };

    Kind kind_ = Kind::Any;
    SemVer lower_;          // inclusive lower bound
    SemVer upper_;          // exclusive upper bound (for Caret/Tilde/Range)
    bool hasUpper_ = false;
    std::string text_;
};

} // namespace mango

#endif
