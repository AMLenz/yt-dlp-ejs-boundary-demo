#include "url_query_utils.h"

#include <iomanip>
#include <sstream>
#include <string>

std::string safe_double_decode(std::string input) {
    auto decode_once = [](const std::string& s) -> std::string {
        std::string out;
        out.reserve(s.size());

        auto hexval = [](char c) -> int {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            return -1;
        };

        for (size_t i = 0; i < s.size(); ++i) {
            if (s[i] == '%' && i + 2 < s.size()) {
                const int hi = hexval(s[i + 1]);
                const int lo = hexval(s[i + 2]);
                if (hi >= 0 && lo >= 0) {
                    out.push_back(static_cast<char>((hi << 4) | lo));
                    i += 2;
                    continue;
                }
            }
            out.push_back(s[i] == '+' ? ' ' : s[i]);
        }
        return out;
    };

    try { input = decode_once(input); } catch (...) {}
    try { input = decode_once(input); } catch (...) {}
    return input;
}

std::map<std::string, std::string> parse_query_string(const std::string& qs) {
    std::map<std::string, std::string> out;
    std::stringstream ss(qs);
    std::string part;

    while (std::getline(ss, part, '&')) {
        const size_t eq = part.find('=');
        if (eq == std::string::npos) {
            out[safe_double_decode(part)] = "";
        } else {
            out[safe_double_decode(part.substr(0, eq))] = safe_double_decode(part.substr(eq + 1));
        }
    }
    return out;
}

std::optional<std::string> query_param_from_url(const std::string& url, const std::string& key) {
    const size_t qpos = url.find('?');
    if (qpos == std::string::npos) return std::nullopt;

    auto params = parse_query_string(url.substr(qpos + 1));
    auto it = params.find(key);
    if (it == params.end()) return std::nullopt;
    return it->second;
}

std::string set_or_replace_query_param(const std::string& url, const std::string& key, const std::string& value) {
    const size_t qpos = url.find('?');
    const std::string base = (qpos == std::string::npos) ? url : url.substr(0, qpos);
    const std::string qs = (qpos == std::string::npos) ? "" : url.substr(qpos + 1);

    auto params = parse_query_string(qs);
    params[key] = value;

    auto urlencode = [](const std::string& s) -> std::string {
        std::ostringstream oss;
        oss << std::hex << std::uppercase;

        for (unsigned char c : s) {
            if ((c >= 'A' && c <= 'Z') ||
                (c >= 'a' && c <= 'z') ||
                (c >= '0' && c <= '9') ||
                c == '-' || c == '_' || c == '.' || c == '~') {
                oss << c;
            } else {
                oss << '%' << std::setw(2) << std::setfill('0') << static_cast<int>(c);
            }
        }
        return oss.str();
    };

    std::ostringstream out;
    bool first = true;
    for (const auto& kv : params) {
        if (!first) out << '&';
        out << urlencode(kv.first) << '=' << urlencode(kv.second);
        first = false;
    }

    return base + "?" + out.str();
}
