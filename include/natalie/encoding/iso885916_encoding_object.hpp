#pragma once

#include <assert.h>
#include <initializer_list>

#include "natalie/encoding_object.hpp"
#include "natalie/string_object.hpp"

namespace Natalie {

using namespace TM;

class Iso885916EncodingObject : public EncodingObject {
public:
    Iso885916EncodingObject()
        : EncodingObject { Encoding::ISO_8859_16, { "ISO-8859-16", "ISO8859-16" } } { }

    virtual bool valid_codepoint(nat_int_t codepoint) const override {
        return (codepoint >= 0 && codepoint <= 0xFF);
    }
    virtual bool in_encoding_codepoint_range(nat_int_t codepoint) override {
        return (codepoint >= 0 && codepoint <= 0xFF);
    }
    virtual bool is_ascii_compatible() const override { return true; };

    virtual std::pair<bool, StringView> prev_char(const String &string, size_t *index) const override;
    virtual std::pair<bool, StringView> next_char(const String &string, size_t *index) const override;

    virtual String escaped_char(unsigned char c) const override;

    virtual nat_int_t to_unicode_codepoint(nat_int_t codepoint) const override;
    virtual nat_int_t from_unicode_codepoint(nat_int_t codepoint) const override;

    virtual String encode_codepoint(nat_int_t codepoint) const override;
    virtual nat_int_t decode_codepoint(StringView &str) const override;
};

}