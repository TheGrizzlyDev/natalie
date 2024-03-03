#include "natalie/global_variable_info/access_hooks.hpp"
#include "natalie.hpp"

namespace Natalie {

namespace GlobalVariableAccessHooks::ReadHooks {

    Value getpid(Env *env, GlobalVariableInfo &info) {
        Object *pid = new IntegerObject { ::getpid() };
        info.set_object(env, pid);
        info.set_read_hook(nullptr);
        return pid;
    }

    Value last_match(Env *env, GlobalVariableInfo &) {
        return env->last_match();
    }

    Value last_match_pre_match(Env *env, GlobalVariableInfo &) {
        auto last_match = env->last_match();
        if (last_match->is_nil())
            return NilObject::the();
        return last_match->as_match_data()->pre_match(env);
    }

    Value last_match_post_match(Env *env, GlobalVariableInfo &) {
        auto last_match = env->last_match();
        if (last_match->is_nil())
            return NilObject::the();
        return last_match->as_match_data()->post_match(env);
    }

    Value last_match_last_group(Env *env, GlobalVariableInfo &) {
        auto last_match = env->last_match();
        if (last_match->is_nil())
            return NilObject::the();
        return last_match->as_match_data()->captures(env)->as_array()->compact(env)->as_array()->last();
    }

}

namespace GlobalVariableAccessHooks::WriteHooks {

    Object *to_int(Env *env, Value v, GlobalVariableInfo &) {
        return v->to_int(env);
    }

    Object *last_match(Env *env, Value v, GlobalVariableInfo &) {
        if (!v || v->is_nil())
            return NilObject::the();
        return v->as_match_data_or_raise(env);
    }
}

}
