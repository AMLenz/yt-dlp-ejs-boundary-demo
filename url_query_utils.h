#pragma once

#include <map>
#include <optional>
#include <string>

std::string safe_double_decode(std::string input);
std::map<std::string, std::string> parse_query_string(const std::string& qs);
std::optional<std::string> query_param_from_url(const std::string& url, const std::string& key);
std::string set_or_replace_query_param(const std::string& url, const std::string& key, const std::string& value);
