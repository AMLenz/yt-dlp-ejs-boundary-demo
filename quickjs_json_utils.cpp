#include "quickjs_json_utils.h"

JSValue json_to_js_qjs(JSContext* ctx, const Json::Value& v) {
    if (v.isNull()) return JS_NULL;
    if (v.isBool()) return JS_NewBool(ctx, v.asBool());
    if (v.isInt()) return JS_NewInt32(ctx, v.asInt());
    if (v.isUInt()) return JS_NewUint32(ctx, v.asUInt());
    if (v.isInt64() || v.isUInt64()) return JS_NewFloat64(ctx, v.asDouble());
    if (v.isDouble()) return JS_NewFloat64(ctx, v.asDouble());
    if (v.isString()) return JS_NewString(ctx, v.asCString());

    if (v.isArray()) {
        JSValue arr = JS_NewArray(ctx);
        for (Json::ArrayIndex i = 0; i < v.size(); ++i) {
            JS_SetPropertyUint32(ctx, arr, i, json_to_js_qjs(ctx, v[i]));
        }
        return arr;
    }

    if (v.isObject()) {
        JSValue obj = JS_NewObject(ctx);
        for (auto it = v.begin(); it != v.end(); ++it) {
            JS_SetPropertyStr(ctx, obj, it.name().c_str(), json_to_js_qjs(ctx, *it));
        }
        return obj;
    }

    return JS_UNDEFINED;
}

Json::Value js_to_json_qjs(JSContext* ctx, JSValueConst val) {
    if (JS_IsNull(val) || JS_IsUndefined(val)) return Json::Value();

    if (JS_IsBool(val)) {
        return Json::Value(static_cast<bool>(JS_ToBool(ctx, val)));
    }

    if (JS_IsNumber(val)) {
        double d = 0.0;
        if (JS_ToFloat64(ctx, &d, val) == 0) return Json::Value(d);
    }

    if (JS_IsString(val)) {
        JsCStringView s(ctx, JS_ToCString(ctx, val));
        if (!s) return Json::Value();
        return Json::Value(s.get());
    }

    if (JS_IsArray(ctx, val)) {
        Json::Value arr(Json::arrayValue);
        uint32_t len = 0;
        JsValueOwner lenv(ctx, JS_GetPropertyStr(ctx, val, "length"));
        JS_ToUint32(ctx, &len, lenv.get());
        for (uint32_t i = 0; i < len; ++i) {
            JsValueOwner elem(ctx, JS_GetPropertyUint32(ctx, val, i));
            arr.append(js_to_json_qjs(ctx, elem.get()));
        }
        return arr;
    }

    if (JS_IsObject(val)) {
        Json::Value obj(Json::objectValue);
        JSPropertyEnum* props = nullptr;
        uint32_t plen = 0;
        if (JS_GetOwnPropertyNames(ctx, &props, &plen, val, JS_GPN_STRING_MASK | JS_GPN_ENUM_ONLY) == 0) {
            for (uint32_t i = 0; i < plen; ++i) {
                JsAtomOwner atom(ctx, props[i].atom);
                JsCStringView key(ctx, JS_AtomToCString(ctx, atom.get()));
                JsValueOwner pv(ctx, JS_GetProperty(ctx, val, atom.get()));
                if (key) obj[key.get()] = js_to_json_qjs(ctx, pv.get());
            }
            js_free(ctx, props);
        }
        return obj;
    }

    return Json::Value();
}

std::string get_quickjs_exception(JSContext* ctx) {
    JsValueOwner exc(ctx, JS_GetException(ctx));
    std::string out = "QuickJS exception";
    JsCStringView msg(ctx, JS_ToCString(ctx, exc.get()));
    if (msg) out = msg.get();
    return out;
}
