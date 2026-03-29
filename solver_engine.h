#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "quickjs_json_utils.h"
#include "yoube_model.h"

void require_regular_file(const std::string& path, std::string_view label);
std::string read_text_file(const std::string& path);

class SolverEngine {
public:
    explicit SolverEngine(const SolverAssets& assets);
    std::vector<SolverResponse> solve(const std::vector<SolverRequest>& requests);

private:
    SolverAssets assets_;
    UniqueJsRuntime rt_;
    UniqueJsContext ctx_;

    void eval_script(const std::string& code, const char* fname);
};

SolverAssets load_solver_assets_from_code(const std::string& base_js_code, const std::string& solver_dir);
SolvedUrlResult solve_format_url(const FormatInfo& fmt, const SolverAssets& assets);
