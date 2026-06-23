#include "mango/Json.hpp"

#include <cctype>
#include <cstdlib>

namespace mango {

const Json* Json::find(std::string_view key) const {
    if (type_ != Type::Object) {
        return nullptr;
    }
    const auto it = object_.find(std::string(key));
    return it == object_.end() ? nullptr : &it->second;
}

class JsonParser {
public:
    JsonParser(std::string_view text, std::string& error) : text_(text), error_(error) {}

    std::optional<Json> parse() {
        skip();
        auto value = parseValue();
        if (!value) {
            return std::nullopt;
        }
        skip();
        if (pos_ != text_.size()) {
            fail("trailing characters after JSON value");
            return std::nullopt;
        }
        return value;
    }

private:
    void fail(const std::string& msg) {
        if (error_.empty()) {
            error_ = "json: " + msg + " at offset " + std::to_string(pos_);
        }
    }

    char peek() const { return pos_ < text_.size() ? text_[pos_] : '\0'; }
    char get() { return text_[pos_++]; }
    bool eof() const { return pos_ >= text_.size(); }

    void skip() {
        for (;;) {
            while (!eof() && std::isspace(static_cast<unsigned char>(peek()))) {
                ++pos_;
            }
            if (peek() == '/' && pos_ + 1 < text_.size() && text_[pos_ + 1] == '/') {
                while (!eof() && peek() != '\n') {
                    ++pos_;
                }
                continue;
            }
            break;
        }
    }

    std::optional<Json> parseValue() {
        skip();
        switch (peek()) {
        case '{': return parseObject();
        case '[': return parseArray();
        case '"': return parseString();
        case 't': case 'f': return parseBool();
        case 'n': return parseNull();
        default: return parseNumber();
        }
    }

    std::optional<Json> parseObject() {
        Json j;
        j.type_ = Json::Type::Object;
        get(); // {
        skip();
        if (peek() == '}') { get(); return j; }
        for (;;) {
            skip();
            if (peek() != '"') { fail("expected string key"); return std::nullopt; }
            auto key = parseRawString();
            if (!key) return std::nullopt;
            skip();
            if (peek() != ':') { fail("expected ':'"); return std::nullopt; }
            get();
            auto value = parseValue();
            if (!value) return std::nullopt;
            j.object_.emplace(std::move(*key), std::move(*value));
            skip();
            if (peek() == ',') { get(); skip(); if (peek() == '}') { get(); return j; } continue; }
            if (peek() == '}') { get(); return j; }
            fail("expected ',' or '}'");
            return std::nullopt;
        }
    }

    std::optional<Json> parseArray() {
        Json j;
        j.type_ = Json::Type::Array;
        get(); // [
        skip();
        if (peek() == ']') { get(); return j; }
        for (;;) {
            auto value = parseValue();
            if (!value) return std::nullopt;
            j.array_.push_back(std::move(*value));
            skip();
            if (peek() == ',') { get(); skip(); if (peek() == ']') { get(); return j; } continue; }
            if (peek() == ']') { get(); return j; }
            fail("expected ',' or ']'");
            return std::nullopt;
        }
    }

    std::optional<std::string> parseRawString() {
        get(); // opening quote
        std::string out;
        while (!eof()) {
            const char c = get();
            if (c == '"') {
                return out;
            }
            if (c == '\\' && !eof()) {
                const char e = get();
                switch (e) {
                case 'n': out += '\n'; break;
                case 't': out += '\t'; break;
                case 'r': out += '\r'; break;
                case '"': out += '"'; break;
                case '\\': out += '\\'; break;
                case '/': out += '/'; break;
                default: out += e; break;
                }
            } else {
                out += c;
            }
        }
        fail("unterminated string");
        return std::nullopt;
    }

    std::optional<Json> parseString() {
        auto s = parseRawString();
        if (!s) return std::nullopt;
        return Json(std::move(*s));
    }

    std::optional<Json> parseBool() {
        if (text_.substr(pos_, 4) == "true") { pos_ += 4; return Json(true); }
        if (text_.substr(pos_, 5) == "false") { pos_ += 5; return Json(false); }
        fail("invalid literal");
        return std::nullopt;
    }

    std::optional<Json> parseNull() {
        if (text_.substr(pos_, 4) == "null") { pos_ += 4; return Json(); }
        fail("invalid literal");
        return std::nullopt;
    }

    std::optional<Json> parseNumber() {
        const std::size_t start = pos_;
        while (!eof()) {
            const char c = peek();
            if (std::isdigit(static_cast<unsigned char>(c)) || c == '-' || c == '+' ||
                c == '.' || c == 'e' || c == 'E') {
                ++pos_;
            } else {
                break;
            }
        }
        if (pos_ == start) { fail("invalid value"); return std::nullopt; }
        // std::strtod is used instead of std::from_chars<double>: the latter's
        // floating-point overload is unimplemented (deleted) in libc++ on macOS.
        const std::string num(text_.substr(start, pos_ - start));
        char* numEnd = nullptr;
        const double value = std::strtod(num.c_str(), &numEnd);
        if (numEnd != num.c_str() + num.size()) { fail("invalid number"); return std::nullopt; }
        return Json(value);
    }

    std::string_view text_;
    std::string& error_;
    std::size_t pos_ = 0;
};

std::optional<Json> Json::parse(std::string_view text, std::string& error) {
    JsonParser parser(text, error);
    return parser.parse();
}

} // namespace mango
