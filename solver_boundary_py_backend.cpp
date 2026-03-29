#include "solver_boundary_py_backend.h"

#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

#include "http_utils.h"
#include "solver_engine.h"

std::vector<SolverRequest> parse_solver_requests_from_json_like(const std::vector<SolverRequest>& requests) {
    return requests;
}

static void validate_requests_or_throw(const std::vector<SolverRequest>& requests) {
    for (const auto& r : requests) {
        if (r.type != "sig" && r.type != "n") {
            throw std::runtime_error("Unsupported request type: " + r.type);
        }
        if (r.challenge.empty()) {
            throw std::runtime_error("Empty challenge for request type: " + r.type);
        }
    }
}

static std::string extract_host_from_watch_url(const std::string& watch_url) {
    std::smatch m;
    if (!std::regex_match(watch_url, m, std::regex("^https://([^/]+)(/.*)$"))) {
        throw std::runtime_error("Unsupported watch_url");
    }
    return m[1].str();
}

static std::string extract_path_from_watch_url(const std::string& watch_url) {
    std::smatch m;
    if (!std::regex_match(watch_url, m, std::regex("^https://([^/]+)(/.*)$"))) {
        throw std::runtime_error("Unsupported watch_url");
    }
    return m[2].str();
}

static std::string extract_js_path_from_watch_html(const std::string& html) {
    std::smatch m;
    if (!std::regex_search(html, m, std::regex("\"jsUrl\":\"([^\"]+)\""))) {
        throw std::runtime_error("jsUrl not found in watch HTML");
    }

    std::string js_path = m[1].str();
    if (js_path.rfind("https://", 0) == 0) {
        std::smatch m2;
        if (!std::regex_match(js_path, m2, std::regex("^https://[^/]+(/.*)$"))) {
            throw std::runtime_error("Unexpected absolute jsUrl format");
        }
        js_path = m2[1].str();
    }

    return js_path;
}

static SolverAssets load_solver_boundary_assets_from_code_core(
    const std::string& player_code,
    const std::string& solver_dir,
    const std::string& meriyah_path,
    const std::string& astring_path
) {
    const std::string lib_path = solver_dir + "/lib.min.js";
    const std::string core_path = solver_dir + "/core.min.js";

    require_regular_file(meriyah_path, "meriyah.umd.js");
    require_regular_file(astring_path, "astring.min.js");
    require_regular_file(lib_path, "lib.min.js");
    require_regular_file(core_path, "core.min.js");

    SolverAssets assets;
    assets.meriyah_code = read_text_file(meriyah_path);
    assets.astring_code = read_text_file(astring_path);
    assets.lib_code = read_text_file(lib_path);
    assets.core_code = read_text_file(core_path);
    assets.player_code = player_code;
    assets.cache = std::make_shared<SolverSharedCache>();
    return assets;
}

static SolverAssets load_solver_boundary_assets_core(
    const std::string& player_path,
    const std::string& solver_dir,
    const std::string& meriyah_path,
    const std::string& astring_path
) {
    require_regular_file(player_path, "player/base.js");

    return load_solver_boundary_assets_from_code_core(
        read_text_file(player_path),
        solver_dir,
        meriyah_path,
        astring_path
    );
}

std::vector<SolverResponse> solve_requests_core(
    const std::string& player_path,
    const std::string& solver_dir,
    const std::string& meriyah_path,
    const std::string& astring_path,
    const std::vector<SolverRequest>& requests
) {
    if (requests.empty()) return {};

    validate_requests_or_throw(requests);

    SolverAssets assets = load_solver_boundary_assets_core(
        player_path,
        solver_dir,
        meriyah_path,
        astring_path
    );

    SolverEngine engine(assets);
    return engine.solve(requests);
}

std::string load_player_code_from_watch_url_core(const std::string& watch_url) {
    const std::string host = extract_host_from_watch_url(watch_url);
    const std::string path = extract_path_from_watch_url(watch_url);

    HttpResponse watch_resp = https_request(
        host,
        "GET",
        path,
        "Accept-Language: en-US,en;q=0.9\r\nReferer: https://www.youtube.com/\r\n",
        ""
    );
    if (watch_resp.status != 200) {
        throw std::runtime_error("watch page returned HTTP " + std::to_string(watch_resp.status));
    }

    const std::string js_path = extract_js_path_from_watch_html(watch_resp.body);

    HttpResponse js_resp = https_request(
        host,
        "GET",
        js_path,
        "Referer: https://www.youtube.com/\r\n",
        ""
    );
    if (js_resp.status != 200) {
        throw std::runtime_error("base.js returned HTTP " + std::to_string(js_resp.status));
    }

    return js_resp.body;
}

std::vector<SolverResponse> solve_requests_from_watch_core(
    const std::string& watch_url,
    const std::string& solver_dir,
    const std::string& meriyah_path,
    const std::string& astring_path,
    const std::vector<SolverRequest>& requests
) {
    if (requests.empty()) return {};

    validate_requests_or_throw(requests);

    const std::string player_code = load_player_code_from_watch_url_core(watch_url);

    SolverAssets assets = load_solver_boundary_assets_from_code_core(
        player_code,
        solver_dir,
        meriyah_path,
        astring_path
    );

    SolverEngine engine(assets);
    return engine.solve(requests);
}
