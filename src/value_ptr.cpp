#include "natalie.hpp"

namespace Natalie {

ValuePtr ValuePtr::public_send(Env *env, SymbolValue *name, size_t argc, ValuePtr *args, Block *block) {
    if (m_type == Type::Integer && IntegerValue::optimized_method(name)) {
        auto synthesized = IntegerValue { m_integer };
        return synthesized._public_send(env, name, argc, args, block);
    }

    return value()->_public_send(env, name, argc, args, block);
}

ValuePtr ValuePtr::send(Env *env, SymbolValue *name, size_t argc, ValuePtr *args, Block *block) {
    if (m_type == Type::Integer && IntegerValue::optimized_method(name)) {
        auto synthesized = IntegerValue { m_integer };
        return synthesized._send(env, name, argc, args, block);
    }

    return value()->_send(env, name, argc, args, block);
}

ValuePtr ValuePtr::send(Env *env, const char *name, size_t argc, ValuePtr *args, Block *block) {
    return send(env, SymbolValue::intern(name), argc, args, block);
}

void ValuePtr::hydrate() {
    switch (m_type) {
    case Type::Integer: {
        m_type = Type::Pointer;
        bool was_gc_enabled = Heap::the().gc_enabled();
        Heap::the().gc_disable();
        m_value = new IntegerValue { m_integer };
        if (was_gc_enabled) Heap::the().gc_enable();
        m_integer = 0;
        break;
    }
    case Type::Pointer:
        break;
    }
}

bool ValuePtr::is_integer() {
    if (m_type == Type::Integer)
        return true;

    return value()->is_integer();
}

bool ValuePtr::is_float() {
    if (m_type == Type::Integer)
        return false;

    return value()->is_float();
}

void ValuePtr::assert_type(Env *env, ValueType type, const char *type_name) {
    if (m_type == Type::Integer && type == ValueType::Integer)
        return;

    value()->assert_type(env, type, type_name);
}

nat_int_t ValuePtr::to_nat_int_t() {
    if (m_type == Type::Integer)
        return m_integer;

    return value()->as_integer()->to_nat_int_t();
}

ValuePtr ValuePtr::try_convert_to_int_or_raise(Env *env) {
    if (is_integer()) {
        return *this;
    }

    auto sym_to_i = SymbolValue::intern("to_i");
    auto sym_to_int = SymbolValue::intern("to_int");

    if (value()->respond_to(env, sym_to_i)) {
        return send(env, sym_to_i);
    } else if (value()->respond_to(env, sym_to_int)) {
        return send(env, sym_to_int);
    } else {
        env->raise("TypeError", "no implicit conversion of {} into Integer", value()->klass()->class_name_or_blank());
    }
}

}
