#pragma once

#include "natalie/forward.hpp"

namespace Natalie {

struct ValuePtr {
    ValuePtr() { }

    ValuePtr(Value *value)
        : m_value { value } { }

    Value &operator*() { return *m_value; }
    Value *operator->() { return m_value; }
    Value *value() { return m_value; }

    bool operator==(Value *other) { return m_value == other; }
    bool operator!=(Value *other) { return m_value != other; }
    bool operator!() { return !m_value; }
    operator bool() { return !!m_value; }

    Value *dummy() { return m_dummy; }

private:
    Value *m_dummy { nullptr }; // flush out any bugs with va_arg(args, Value*)

    Value *m_value { nullptr };
};

}
