#include "natalie.hpp"

namespace Natalie {

Value Args::operator[](size_t index) const {
    // TODO: remove the assertion here once we're
    // in a good place with the transition to Args
    assert(index < argc);
    return args[index];
}

Value Args::at(size_t index) const {
    assert(index < argc);
    return args[index];
}

Value Args::at(size_t index, Value default_value) const {
    if (index >= argc)
        return default_value;
    return args[index];
}

Args::Args(ArrayObject &a)
    : argc { a.size() }
    , args { a.data() } { }

Args::Args(const Args &other)
    : argc { other.argc } {
    args = other.args;
}

Args &Args::operator=(const Args &other) {
    argc = other.argc;
    args = other.args;
    return *this;
}

Args Args::shift(Args &args) {
    return Args { args.argc - 1, args.args + 1 };
}

ArrayObject *Args::to_array() const {
    return new ArrayObject { argc, args };
}

ArrayObject *Args::to_array_for_block(Env *env) const {
    if (argc == 1)
        return to_ary(env, args[0], true)->dup(env)->as_array();
    return new ArrayObject { argc, args };
}

void Args::ensure_argc_is(Env *env, size_t expected) const {
    if (argc != expected)
        env->raise("ArgumentError", "wrong number of arguments (given {}, expected {})", argc, expected);
}

void Args::ensure_argc_between(Env *env, size_t expected_low, size_t expected_high) const {
    if (argc < expected_low || argc > expected_high)
        env->raise("ArgumentError", "wrong number of arguments (given {}, expected {}..{})", argc, expected_low, expected_high);
}

void Args::ensure_argc_at_least(Env *env, size_t expected) const {
    if (argc < expected)
        env->raise("ArgumentError", "wrong number of arguments (given {}, expected {}+)", argc, expected);
}

}