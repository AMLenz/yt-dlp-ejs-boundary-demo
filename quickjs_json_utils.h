#pragma once

#include <json/json.h>
#include <string>

#include "quickjs_raii.h"

JSValue json_to_js_qjs(JSContext* ctx, const Json::Value& v);
Json::Value js_to_json_qjs(JSContext* ctx, JSValueConst val);
std::string get_quickjs_exception(JSContext* ctx);
