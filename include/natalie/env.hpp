#pragma once

#include <memory>
#include <stdlib.h>

#include "natalie/forward.hpp"
#include "natalie/gc.hpp"
#include "natalie/global_env.hpp"
#include "natalie/local_jump_error_type.hpp"
#include "natalie/managed_vector.hpp"
#include "natalie/value.hpp"

namespace Natalie {

using namespace TM;

extern thread_local ExceptionObject *tl_current_exception;

class Env : public Cell {
public:
    static Env *create() {
        std::lock_guard<std::recursive_mutex> lock(g_gc_recursive_mutex);
        return new Env;
    }

    static Env *create(Env *outer) {
        std::lock_guard<std::recursive_mutex> lock(g_gc_recursive_mutex);
        return new Env { outer };
    }

    static Env *create(const Env &other) {
        std::lock_guard<std::recursive_mutex> lock(g_gc_recursive_mutex);
        return new Env { other };
    }

    Env() { }

    Env(Env *outer)
        : m_outer { outer } { }

    Env(const Env &other)
        : m_vars { other.m_vars }
        , m_outer { other.m_outer }
        , m_block { other.m_block }
        , m_caller { nullptr }
        , m_file { other.m_file }
        , m_line { other.m_line }
        , m_method { other.m_method }
        , m_module { other.m_module } { }

    Env &operator=(Env &other) = delete;

    virtual ~Env() override { }

    bool global_defined(SymbolObject *);
    Value global_get(SymbolObject *);
    Value global_set(SymbolObject *, Object *, bool = false);
    Value global_set(SymbolObject *, Optional<Value>, bool = false);
    Value global_alias(SymbolObject *, SymbolObject *);

    const Method *current_method();
    String build_code_location_name();

    Value var_get(const char *, size_t);
    Value var_set(const char *, size_t, bool, Value);

    [[noreturn]] void raise(ClassObject *, StringObject *);
    [[noreturn]] void raise(ClassObject *, String);
    [[noreturn]] void raise(const char *, String);
    [[noreturn]] void raise_exception(ExceptionObject *);
    [[noreturn]] void raise_key_error(Value, Value);
    [[noreturn]] void raise_local_jump_error(Value, LocalJumpErrorType, nat_int_t break_point = 0);
    [[noreturn]] void raise_errno();
    [[noreturn]] void raise_errno(int);
    [[noreturn]] void raise_errno(StringObject *);
    [[noreturn]] void raise_invalid_byte_sequence_error(const EncodingObject *);
    [[noreturn]] void raise_no_method_error(Value, SymbolObject *, MethodMissingReason);
    [[noreturn]] void raise_name_error(SymbolObject *name, String);
    [[noreturn]] void raise_name_error(StringObject *name, String);
    [[noreturn]] void raise_not_comparable_error(Value lhs, Value rhs);

    // Old error message style, e.g.:
    // - no implicit conversion from nil to string
    // - no implicit conversion of Integer into String
    [[noreturn]] void raise_type_error(Value obj, const char *expected);

    // New error message style, e.g.:
    // - no implicit conversion of nil into String
    // - no implicit conversion of Integer into String
    [[noreturn]] void raise_type_error2(Value obj, const char *expected);

    template <typename... Args>
    [[noreturn]] void raise_name_error(SymbolObject *name, const char *format, Args... args) {
        auto message = String::format(format, args...);
        raise_name_error(name, message);
    }

    template <typename... Args>
    [[noreturn]] void raise_name_error(StringObject *name, const char *format, Args... args) {
        auto message = String::format(format, args...);
        raise_name_error(name, message);
    }

    template <typename... Args>
    [[noreturn]] void raise(ClassObject *klass, const char *format, Args... args) {
        auto message = String::format(format, args...);
        raise(klass, message);
    }

    template <typename... Args>
    [[noreturn]] void raise(const char *class_name, const char *format, Args... args) {
        auto message = String::format(format, args...);
        raise(class_name, message);
    }

    void set_catch(Value value) { m_catch = value; }
    bool has_catch(Value value) const;

    void warn(String);

    template <typename... Args>
    void warn(const char *format, Args... args) {
        auto message = String::format(format, args...);
        warn(message);
    }

    template <typename... Args>
    void deprecation_warn(const char *format, Args... args) {
        if (GlobalEnv::the()->show_deprecation_warnings(this))
            warn(String::format(format, args...));
    }

    template <typename... Args>
    void verbose_warn(const char *format, Args... args) {
        if (GlobalEnv::the()->is_verbose())
            warn(String::format(format, args...));
    }

    void ensure_block_given(Block *);
    void ensure_no_missing_keywords(HashObject *, std::initializer_list<const String>);
    void ensure_no_missing_keywords(Value kwargs, std::initializer_list<const String> list) { return ensure_no_missing_keywords(kwargs.as_hash(), list); }
    void ensure_no_extra_keywords(HashObject *);
    void ensure_no_extra_keywords(Value kwargs) { return ensure_no_extra_keywords(kwargs.as_hash()); }

    Value last_match();
    bool has_last_match();
    void set_last_match(MatchDataObject *match);

    void build_vars(size_t);

    Env *outer() { return m_outer; }
    void clear_outer() { m_outer = nullptr; }

    Env *caller() { return m_caller; }
    void set_caller(Env *caller) { m_caller = caller; }

    Env *non_block_env();

    Block *nearest_block(bool allow_null = false) {
        Env *env_with_block = this;
        while (!env_with_block->block() && env_with_block->outer())
            env_with_block = env_with_block->outer();
        if (!env_with_block->block()) {
            if (allow_null)
                return nullptr;
            else
                raise("LocalJumpError", "no block given");
        }
        return env_with_block->block();
    }

    Block *block() { return m_block; }
    void set_block(Block *block) { m_block = block; }

    Block *this_block() { return m_this_block; }
    void set_this_block(Block *block) { m_this_block = block; }

    const char *file() const { return m_file; }
    void set_file(const char *file) { m_file = file; }

    size_t line() const { return m_line; }
    void set_line(size_t line) { m_line = line; }

    const Method *method() { return m_method; }
    void set_method(const Method *method) { m_method = method; }

    ModuleObject *module() { return m_module; }
    void set_module(ModuleObject *module) { m_module = module; }

    Optional<Value> match() { return m_match; }
    void set_match(Optional<Value> match) { m_match = match; }
    void clear_match() { m_match = Optional<Value>(); }

    Value exception_object();
    ExceptionObject *exception();
    void set_exception(ExceptionObject *exception) { m_exception = exception; }
    void clear_exception() { m_exception = nullptr; }

    Backtrace *backtrace();

    bool is_main() { return this == GlobalEnv::the()->main_env(); }

    virtual void visit_children(Visitor &visitor) const override final;

    virtual TM::String dbg_inspect(int indent = 0) const override {
        return TM::String::format("<Env {h} outer={h}>", this, m_outer);
    }

    Value output_file_separator();
    Value output_record_separator();
    Value last_line();
    Value set_last_line(Value);
    Value set_last_lineno(Value);

private:
    ManagedVector<Value> *m_vars { nullptr };
    Env *m_outer { nullptr };
    Block *m_block { nullptr };
    Block *m_this_block { nullptr };
    Env *m_caller { nullptr };
    const char *m_file { nullptr };
    size_t m_line { 0 };
    const Method *m_method { nullptr };
    ModuleObject *m_module { nullptr };
    Optional<Value> m_match {};
    ExceptionObject *m_exception { nullptr };
    Optional<Value> m_catch {};
};
}
