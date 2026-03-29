#pragma once

#include <json/json.h>
#include <string>
#include <vector>

std::string trim(const std::string& s);
std::string read_line_prompt(const std::string& prompt);
std::vector<size_t> parse_selection_indices(const std::string& input, size_t max_index);

std::string get_string_safe(const Json::Value& obj, const char* key);
int get_int_safe(const Json::Value& obj, const char* key);
