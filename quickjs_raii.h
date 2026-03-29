#pragma once

#include <memory>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wcast-function-type"
#include <quickjs.h>
#pragma GCC diagnostic pop

struct JsRuntimeDeleter {
    void operator()(JSRuntime* ptr) const noexcept {
        if (ptr) JS_FreeRuntime(ptr);
    }
};

struct JsContextDeleter {
    void operator()(JSContext* ptr) const noexcept {
        if (ptr) JS_FreeContext(ptr);
    }
};

struct JsValueOwner {
    JSContext* ctx = nullptr;
    JSValue value = JS_UNDEFINED;

    JsValueOwner() = default;
    JsValueOwner(JSContext* context, JSValue v) noexcept : ctx(context), value(v) {}

    JsValueOwner(const JsValueOwner&) = delete;
    JsValueOwner& operator=(const JsValueOwner&) = delete;

    JsValueOwner(JsValueOwner&& other) noexcept : ctx(other.ctx), value(other.value) {
        other.ctx = nullptr;
        other.value = JS_UNDEFINED;
    }

    JsValueOwner& operator=(JsValueOwner&& other) noexcept {
        if (this != &other) {
            reset();
            ctx = other.ctx;
            value = other.value;
            other.ctx = nullptr;
            other.value = JS_UNDEFINED;
        }
        return *this;
    }

    ~JsValueOwner() { reset(); }

    [[nodiscard]] JSValueConst get() const noexcept { return value; }

    void reset(JSContext* new_ctx = nullptr, JSValue new_value = JS_UNDEFINED) noexcept {
        if (ctx) JS_FreeValue(ctx, value);
        ctx = new_ctx;
        value = new_value;
    }
};

struct JsCStringView {
    JSContext* ctx = nullptr;
    const char* value = nullptr;

    JsCStringView() = default;
    JsCStringView(JSContext* context, const char* cstr) noexcept : ctx(context), value(cstr) {}

    JsCStringView(const JsCStringView&) = delete;
    JsCStringView& operator=(const JsCStringView&) = delete;

    JsCStringView(JsCStringView&& other) noexcept : ctx(other.ctx), value(other.value) {
        other.ctx = nullptr;
        other.value = nullptr;
    }

    JsCStringView& operator=(JsCStringView&& other) noexcept {
        if (this != &other) {
            reset();
            ctx = other.ctx;
            value = other.value;
            other.ctx = nullptr;
            other.value = nullptr;
        }
        return *this;
    }

    ~JsCStringView() { reset(); }

    [[nodiscard]] const char* get() const noexcept { return value; }
    explicit operator bool() const noexcept { return value != nullptr; }

    void reset(JSContext* new_ctx = nullptr, const char* new_value = nullptr) noexcept {
        if (ctx && value) JS_FreeCString(ctx, value);
        ctx = new_ctx;
        value = new_value;
    }
};

struct JsAtomOwner {
    JSContext* ctx = nullptr;
    JSAtom atom = JS_ATOM_NULL;

    JsAtomOwner() = default;
    JsAtomOwner(JSContext* context, JSAtom a) noexcept : ctx(context), atom(a) {}

    JsAtomOwner(const JsAtomOwner&) = delete;
    JsAtomOwner& operator=(const JsAtomOwner&) = delete;

    JsAtomOwner(JsAtomOwner&& other) noexcept : ctx(other.ctx), atom(other.atom) {
        other.ctx = nullptr;
        other.atom = JS_ATOM_NULL;
    }

    JsAtomOwner& operator=(JsAtomOwner&& other) noexcept {
        if (this != &other) {
            reset();
            ctx = other.ctx;
            atom = other.atom;
            other.ctx = nullptr;
            other.atom = JS_ATOM_NULL;
        }
        return *this;
    }

    ~JsAtomOwner() { reset(); }

    [[nodiscard]] JSAtom get() const noexcept { return atom; }

    void reset(JSContext* new_ctx = nullptr, JSAtom new_atom = JS_ATOM_NULL) noexcept {
        if (ctx && atom != JS_ATOM_NULL) JS_FreeAtom(ctx, atom);
        ctx = new_ctx;
        atom = new_atom;
    }
};

using UniqueJsRuntime = std::unique_ptr<JSRuntime, JsRuntimeDeleter>;
using UniqueJsContext = std::unique_ptr<JSContext, JsContextDeleter>;
