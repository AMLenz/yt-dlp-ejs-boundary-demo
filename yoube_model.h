#pragma once

#include <json/json.h>

#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

struct SolverSharedCache {
    std::mutex mutex;
    std::unordered_map<std::string, std::string> n_values;
    std::unordered_map<std::string, std::string> sig_values;
    std::unordered_map<std::string, std::string> n_errors;
    std::unordered_map<std::string, std::string> sig_errors;
};

struct SolverAssets {
    std::string meriyah_code;
    std::string astring_code;
    std::string lib_code;
    std::string core_code;
    std::string player_code;
    std::shared_ptr<SolverSharedCache> cache;
};

struct SolverRequest {
    std::string type;
    std::string challenge;
};

struct SolverResponse {
    std::string type;
    bool ok = false;
    std::string data;
    std::string error;
};

struct SolvedUrlResult {
    std::string decoded_url;
    std::string solver_error;
};

struct FormatInfo {
    int itag = 0;
    std::string mimeType;
    std::string container;
    std::string codecs;
    bool hasVideo = false;
    bool hasAudio = false;
    bool hasDirectUrl = false;
    bool hasSignatureCipher = false;
    int width = 0;
    int height = 0;
    int fps = 0;
    int bitrate = 0;
    std::string qualityLabel;
    std::string url;
    std::string signatureCipher;
    int audioChannels = 0;
    int audioSampleRate = 0;

    std::string sp = "signature";
    std::string s;
    std::string n;
    std::vector<SolverRequest> requests;
    std::string audioTrackId;
std::string audioTrackDisplayName;
std::string audioLanguage;
bool audioIsDefault = false;
bool audioIsDubbed = false;
};

struct HttpResponse {
    int status = 0;
    std::string headers, body;
};

struct BrowseMeta {
    std::string apiKey, clientVersion, visitorData;
};

struct VideoEntry {
    std::string videoId;
    std::string title;
};

struct TvHtml5Context {
    std::map<std::string, std::string> cookies;
    std::string cookie_header;
    std::string sapisid;
    std::string api_key;
    std::string js_url;
    std::string base_js;
    SolverAssets solver;
    int sts = 0;
    Json::Value player_json;
};
