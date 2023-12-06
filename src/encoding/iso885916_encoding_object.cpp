#include "natalie.hpp"

namespace Natalie {

static const long ISO885916[] = {
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
    0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 0x90, 0x91, 0x92, 0x93,
    0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D,
    0x9E, 0x9F, 0xA0, 0x104, 0x105, 0x141, 0x20AC, 0x201E, 0x160, 0xA7,
    0x161, 0xA9, 0x218, 0xAB, 0x179, 0xAD, 0x17A, 0x17B, 0xB0, 0xB1,
    0x10C, 0x142, 0x17D, 0x201D, 0xB6, 0xB7, 0x17E, 0x10D, 0x219, 0xBB,
    0x152, 0x153, 0x178, 0x17C, 0xC0, 0xC1, 0xC2, 0x102, 0xC4, 0x106,
    0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
    0x110, 0x143, 0xD2, 0xD3, 0xD4, 0x150, 0xD6, 0x15A, 0x170, 0xD9,
    0xDA, 0xDB, 0xDC, 0x118, 0x21A, 0xDF, 0xE0, 0xE1, 0xE2, 0x103,
    0xE4, 0x107, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED,
    0xEE, 0xEF, 0x111, 0x144, 0xF2, 0xF3, 0xF4, 0x151, 0xF6, 0x15B,
    0x171, 0xF9, 0xFA, 0xFB, 0xFC, 0x119, 0x21B, 0xFF
};
static const long ISO885916_max = 127;

std::pair<bool, StringView> Iso885916EncodingObject::prev_char(const String &string, size_t *index) const {
    if (*index == 0)
        return { true, StringView() };
    (*index)--;
    return { true, StringView(&string, *index, 1) };
}

std::pair<bool, StringView> Iso885916EncodingObject::next_char(const String &string, size_t *index) const {
    if (*index >= string.size())
        return { true, StringView() };
    size_t i = *index;
    (*index)++;
    return { true, StringView(&string, i, 1) };
}

String Iso885916EncodingObject::escaped_char(unsigned char c) const {
    char buf[5];
    snprintf(buf, 5, "\\x%02llX", (long long)c);
    return String(buf);
}

nat_int_t Iso885916EncodingObject::to_unicode_codepoint(nat_int_t codepoint) const {
    if (codepoint >= 0x00 && codepoint <= 0x7F)
        return codepoint;
    if (codepoint >= 0x80 && codepoint <= 0xFF)
        return ISO885916[codepoint - 0x80];
    return -1;
}

nat_int_t Iso885916EncodingObject::from_unicode_codepoint(nat_int_t codepoint) const {
    if (codepoint >= 0x00 && codepoint <= 0x7F)
        return codepoint;
    for (long i = 0; i <= ISO885916_max; i++) {
        if (ISO885916[i] == codepoint)
            return i + 0x80;
    }
    return -1;
}

String Iso885916EncodingObject::encode_codepoint(nat_int_t codepoint) const {
    return String((char)codepoint);
}

nat_int_t Iso885916EncodingObject::decode_codepoint(StringView &str) const {
    switch (str.size()) {
    case 1:
        return (unsigned char)str[0];
    default:
        return -1;
    }
}

}
