#pragma once

#include <string>
#include <vector>

#include "yoube_model.h"

std::vector<SolverRequest> parse_solver_requests_from_json_like(const std::vector<SolverRequest>& requests);

std::vector<SolverResponse> solve_requests_core(
    const std::string& player_path,
    const std::string& solver_dir,
    const std::string& meriyah_path,
    const std::string& astring_path,
    const std::vector<SolverRequest>& requests
);

std::string load_player_code_from_watch_url_core(const std::string& watch_url);

std::vector<SolverResponse> solve_requests_from_watch_core(
    const std::string& watch_url,
    const std::string& solver_dir,
    const std::string& meriyah_path,
    const std::string& astring_path,
    const std::vector<SolverRequest>& requests
);
