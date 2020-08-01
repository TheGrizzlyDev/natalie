#pragma once

#include <assert.h>
#include <float.h>
#include <math.h>

#include "natalie/class_value.hpp"
#include "natalie/forward.hpp"
#include "natalie/global_env.hpp"
#include "natalie/integer_value.hpp"
#include "natalie/macros.hpp"
#include "natalie/value.hpp"

namespace Natalie {

struct FloatValue : Value {
    FloatValue(Env *env, double number)
        : Value { Value::Type::Float, env->Object()->const_get(env, "Float", true)->as_class() }
        , m_float { number } { }

    FloatValue(Env *env, int64_t number)
        : Value { Value::Type::Float, env->Object()->const_get(env, "Float", true)->as_class() }
        , m_float { static_cast<double>(number) } { }

    FloatValue(const FloatValue &other)
        : Value { Value::Type::Float, const_cast<FloatValue &>(other).klass() }
        , m_float { other.m_float } { }

    static FloatValue *nan(Env *env) {
        return new FloatValue { env, 0.0 / 0.0 };
    }

    static FloatValue *positive_infinity(Env *env) {
        return new FloatValue { env, std::numeric_limits<double>::infinity() };
    }

    static FloatValue *negative_infinity(Env *env) {
        return new FloatValue { env, -std::numeric_limits<double>::infinity() };
    }

    static FloatValue *max(Env *env) {
        return new FloatValue { env, DBL_MAX };
    }

    static FloatValue *neg_max(Env *env) {
        return new FloatValue { env, -DBL_MAX };
    }

    static FloatValue *min(Env *env) {
        return new FloatValue { env, DBL_MIN };
    }

    double to_double() {
        return m_float;
    }

    Value *to_int_no_truncation(Env *env) {
        if (is_nan() || is_infinity()) return this;
        if (m_float == ::floor(m_float)) {
            return new IntegerValue { env, static_cast<int64_t>(m_float) };
        }
        return this;
    }

    bool is_zero() {
        return m_float == 0 && !is_nan();
    }

    bool is_finite() {
        return !(is_negative_infinity() || is_positive_infinity() || is_nan());
    }

    bool is_nan() {
        return isnan(m_float);
    }

    bool is_infinity() {
        return isinf(m_float);
    }

    bool is_negative() {
        return m_float < 0.0;
    };

    bool is_positive() {
        return m_float > 0.0;
    };

    // NOTE: even though this is a predicate method with a ? suffix, it returns an 1, -1, or nil.
    Value *is_infinite(Env *);

    bool is_positive_infinity() {
        return is_infinity() && m_float > 0;
    }

    bool is_negative_infinity() {
        return is_infinity() && m_float < 0;
    }

    FloatValue *negate() {
        FloatValue *copy = new FloatValue { *this };
        copy->m_float *= -1;
        return copy;
    }

    bool eq(Env *, Value *);

    bool eql(Value *);

    Value *to_s(Env *);
    Value *cmp(Env *, Value *);
    Value *coerce(Env *, Value *);
    Value *to_i(Env *);
    Value *to_f() { return this; }
    Value *add(Env *, Value *);
    Value *sub(Env *, Value *);
    Value *mul(Env *, Value *);
    Value *div(Env *, Value *);
    Value *mod(Env *, Value *);
    Value *divmod(Env *, Value *);
    Value *pow(Env *, Value *);
    Value *abs(Env *);
    Value *ceil(Env *, Value *);
    Value *floor(Env *, Value *);
    Value *round(Env *, Value *);
    Value *truncate(Env *, Value *);
    Value *next_float(Env *);
    Value *prev_float(Env *);
    bool lt(Env *, Value *);
    bool lte(Env *, Value *);
    bool gt(Env *, Value *);
    bool gte(Env *, Value *);

    Value *uminus() {
        return negate();
    }

    Value *uplus() {
        return this;
    }

    static void build_constants(Env *env, ClassValue *klass) {
        klass->const_set(env, "DIG", new FloatValue { env, double { DBL_DIG } });
        klass->const_set(env, "EPSILON", new FloatValue { env, std::numeric_limits<double>::epsilon() });
        klass->const_set(env, "INFINITY", FloatValue::positive_infinity(env));
        klass->const_set(env, "MANT_DIG", new FloatValue { env, double { DBL_MANT_DIG } });
        klass->const_set(env, "MAX", FloatValue::max(env));
        klass->const_set(env, "MAX_10_EXP", new FloatValue { env, double { DBL_MAX_10_EXP } });
        klass->const_set(env, "MAX_EXP", new FloatValue { env, double { DBL_MAX_EXP } });
        klass->const_set(env, "MIN", FloatValue::min(env));
        klass->const_set(env, "MIN_10_EXP", new FloatValue { env, double { DBL_MIN_10_EXP } });
        klass->const_set(env, "MIN_EXP", new FloatValue { env, double { DBL_MIN_EXP } });
        klass->const_set(env, "NAN", FloatValue::nan(env));
        klass->const_set(env, "RADIX", new FloatValue { env, double { std::numeric_limits<double>::radix } });
    }

private:
    double m_float { 0.0 };
};
}
