#pragma once

#include <assert.h>

#include "natalie/block.hpp"
#include "natalie/class_object.hpp"
#include "natalie/forward.hpp"
#include "natalie/global_env.hpp"
#include "natalie/macros.hpp"
#include "natalie/object.hpp"

namespace Natalie {

struct HashKey : public Cell {
    static HashKey *create(Value key, Value val, size_t hash) {
        std::lock_guard<std::recursive_mutex> lock(g_gc_recursive_mutex);
        return new HashKey { key, val, hash };
    }

    HashKey *prev { nullptr };
    HashKey *next { nullptr };
    Value key;
    Value val;
    size_t hash { 0 };
    bool removed { false };

    virtual void visit_children(Visitor &visitor) const override final {
        visitor.visit(prev);
        visitor.visit(next);
        visitor.visit(key);
        visitor.visit(val);
    }

    virtual TM::String dbg_inspect(int indent = 0) const override {
        return TM::String::format("<HashKey {h} key={} val={}>", this, key.dbg_inspect(), val.dbg_inspect());
    }

    HashKey() { }

    HashKey(Value key, Value val, size_t hash)
        : key { key }
        , val { val }
        , hash { hash } { }
};

}

namespace TM {

template <>
struct HashKeyHandler<Natalie::HashKey *> {
    static size_t hash(Natalie::HashKey *);
    static bool compare(Natalie::HashKey *, Natalie::HashKey *, void *);
};

}

namespace Natalie {

class HashObject : public Object {
public:
    static HashObject *create() {
        std::lock_guard<std::recursive_mutex> lock(g_gc_recursive_mutex);
        return new HashObject();
    }

    static HashObject *create(ClassObject *klass) {
        std::lock_guard<std::recursive_mutex> lock(g_gc_recursive_mutex);
        return new HashObject(klass);
    }

    static HashObject *create(Env *env, std::initializer_list<Value> items) {
        std::lock_guard<std::recursive_mutex> lock(g_gc_recursive_mutex);
        return new HashObject(env, items);
    }

    static HashObject *create(Env *env, size_t argc, Value *items) {
        std::lock_guard<std::recursive_mutex> lock(g_gc_recursive_mutex);
        return new HashObject(env, argc, items);
    }

    static HashObject *create(Env *env, const HashObject &other) {
        std::lock_guard<std::recursive_mutex> lock(g_gc_recursive_mutex);
        return new HashObject(env, other);
    }

    static Value square_new(Env *, ClassObject *klass, Args &&args);

    static Value size_fn(Env *env, Value self, Args &&, Block *) {
        return self.as_hash()->size(env);
    }

    static bool is_ruby2_keywords_hash(Env *, Value);
    static Value ruby2_keywords_hash(Env *, Value);

    size_t size() const { return m_hashmap.size(); }
    Value size(Env *) const;

    bool is_empty() { return m_hashmap.size() == 0; }

    Optional<Value> get(Env *, Value);
    Value get_default(Env *, Optional<Value> = {});
    Value set_default(Env *, Value);

    void put(Env *, Value, Value);
    Optional<Value> remove(Env *, Value);
    Value clear(Env *);

    Value default_proc(Env *);
    Value set_default_proc(Env *, Value);
    void set_default_proc(ProcObject *proc) { m_default_proc = proc; }

    Value default_value() { return m_default_value; }

    Value compact(Env *);
    Value compact_in_place(Env *);

    bool is_iterating() const { return m_is_iterating; }
    void set_is_iterating(bool is_iterating) { m_is_iterating = is_iterating; }

    class iterator {
    public:
        iterator(HashKey *key, const HashObject *hash)
            : m_key { key }
            , m_hash { hash } { }

        iterator operator++() {
            if (m_key->next == nullptr || (!m_key->removed && m_key->next == m_hash->m_key_list)) {
                m_key = nullptr;
            } else if (m_key->next->removed) {
                m_key = m_key->next;
                return operator++();
            } else {
                m_key = m_key->next;
            }
            return *this;
        }

        iterator operator++(int _) {
            iterator i = *this;
            operator++();
            return i;
        }

        HashKey &operator*() { return *m_key; }
        HashKey *operator->() { return m_key; }

        friend bool operator==(const iterator &i1, const iterator &i2) {
            return i1.m_key == i2.m_key;
        }

        friend bool operator!=(const iterator &i1, const iterator &i2) {
            return i1.m_key != i2.m_key;
        }

    private:
        // cannot be heap-allocated, because the GC is not aware of it.
        void *operator new(size_t size) = delete;

        HashKey *m_key;
        const HashObject *m_hash;
    };

    iterator begin() const {
        return iterator { m_key_list, this };
    }

    iterator end() const {
        return iterator { nullptr, this };
    }

    Value compare_by_identity(Env *);
    bool is_comparing_by_identity() const;
    Value delete_if(Env *, Block *);
    Value delete_key(Env *, Value, Block *);
    Value dig(Env *, Args &&);
    Value each(Env *, Block *);
    bool eq(Env *, Value, SymbolObject *);
    bool eq(Env *, Value);
    bool eql(Env *, Value);
    bool gte(Env *, Value);
    bool gt(Env *, Value);
    bool lte(Env *, Value);
    bool lt(Env *, Value);
    Value except(Env *, Args &&);
    Value fetch(Env *, Value, Optional<Value> = {}, Block * = nullptr);
    Value fetch_values(Env *, Args &&, Block *);
    Value hash(Env *);
    bool has_key(Env *, Value);
    bool has_value(Env *, Value);
    Value initialize(Env *, Optional<Value>, Optional<Value> = {}, Block * = nullptr);
    Value inspect(Env *);
    Value keep_if(Env *, Block *);
    Value keys(Env *);
    Value merge(Env *, Args &&, Block *);
    Value merge_in_place(Env *, Args &&, Block *);
    Value ref(Env *, Value);
    Value refeq(Env *, Value, Value);
    Value slice(Env *, Args &&);
    Value replace(Env *, Value);
    Value rehash(Env *);
    Value values(Env *);

    Value to_h(Env *, Block *);
    Value to_hash() { return this; }

    virtual void visit_children(Visitor &) const override final;

    virtual String dbg_inspect(int indent = 0) const override;

    virtual bool is_large() override {
        return m_hashmap.capacity() >= 100;
    }

private:
    HashObject()
        : HashObject { GlobalEnv::the()->Hash() } { }

    HashObject(Env *env, std::initializer_list<Value> items)
        : HashObject {} {
        assert(items.size() % 2 == 0);
        for (auto it = items.begin(); it != items.end(); it++) {
            auto key = *it;
            it++;
            auto value = *it;
            put(env, key, value);
        }
    }

    HashObject(Env *env, size_t argc, Value *items)
        : HashObject {} {
        assert(argc % 2 == 0);
        for (size_t i = 0; i < argc; i += 2) {
            auto key = items[i];
            auto value = items[i + 1];
            put(env, key, value);
        }
    }

    HashObject(ClassObject *klass)
        : Object { Object::Type::Hash, klass }
        , m_default_value { Value::nil() } { }

    HashObject(Env *env, const HashObject &other)
        : Object { other }
        , m_is_comparing_by_identity { other.m_is_comparing_by_identity }
        , m_default_value { other.m_default_value }
        , m_default_proc { other.m_default_proc } {
        for (auto node : other) {
            put(env, node.key, node.val);
        }
    }

    HashObject &operator=(HashObject &&other) {
        Object::operator=(std::move(other));
        m_hashmap.clear();
        m_hashmap = std::move(other.m_hashmap);
        m_key_list = other.m_key_list;
        m_is_comparing_by_identity = other.m_is_comparing_by_identity;
        m_default_value = other.m_default_value;
        m_default_proc = other.m_default_proc;
        other.m_key_list = nullptr;
        return *this;
    }

    void key_list_remove_node(HashKey *);
    HashKey *key_list_append(Env *, Value, nat_int_t, Value);
    nat_int_t generate_key_hash(Env *, Value) const;

    void destroy_key_list() {
        if (!m_key_list) return;
        HashKey *first_key = m_key_list;
        HashKey *key = m_key_list;
        do {
            HashKey *next_key = key->next;
            delete key;
            key = next_key;
        } while (key != first_key);
    }

    HashKey *m_key_list { nullptr };
    TM::Hashmap<HashKey *, Optional<Value>> m_hashmap { 10 }; // TODO: profile and tune this initial capacity
    bool m_is_iterating { false };
    bool m_is_comparing_by_identity { false };
    bool m_is_ruby2_keywords_hash { false };
    Value m_default_value {};
    ProcObject *m_default_proc { nullptr };
};
}
