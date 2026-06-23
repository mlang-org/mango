#ifndef MANGO_JSON_HPP
#define MANGO_JSON_HPP

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace mango {

// A minimal JSON value and parser, sufficient for reading mango.json manifests
// and lockfiles. Object key order is not preserved (std::map), which is fine for
// manifests. Tolerates trailing commas and // line comments on read.
class Json {
public:
    enum class Type { Null, Bool, Number, String, Array, Object };

    Json() = default;
    explicit Json(bool b) : type_(Type::Bool), bool_(b) {}
    explicit Json(double n) : type_(Type::Number), number_(n) {}
    explicit Json(std::string s) : type_(Type::String), string_(std::move(s)) {}

    Type type() const { return type_; }
    bool isObject() const { return type_ == Type::Object; }
    bool isArray() const { return type_ == Type::Array; }
    bool isString() const { return type_ == Type::String; }

    const std::string& asString() const { return string_; }
    double asNumber() const { return number_; }
    bool asBool() const { return bool_; }

    const std::map<std::string, Json>& object() const { return object_; }
    const std::vector<Json>& array() const { return array_; }

    // Returns the named member of an object, or nullptr.
    const Json* find(std::string_view key) const;

    static std::optional<Json> parse(std::string_view text, std::string& error);

private:
    Type type_ = Type::Null;
    bool bool_ = false;
    double number_ = 0.0;
    std::string string_;
    std::vector<Json> array_;
    std::map<std::string, Json> object_;

    friend class JsonParser;
};

} // namespace mango

#endif
