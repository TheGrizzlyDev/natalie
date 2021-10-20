#pragma once

#include "natalie/string.hpp"

namespace Natalie {

namespace ArrayPacker {

    struct Token {
        signed char directive { 0 };
        int count { -1 };
        bool star { false };
        String *error { nullptr };
    };

    class Tokenizer {
    public:
        Tokenizer(String *directives)
            : m_directives { directives } { }

        TM::Vector<Token> *tokenize() {
            auto tokens = new TM::Vector<Token> {};
            for (Token token = next_token(); token.directive; token = next_token()) {
                tokens->push(token);
                if (token.error)
                    break;
            }
            return tokens;
        }

    private:
        Token next_token() {
            auto d = next_char();
            if (!d) return {};

            auto token = Token { d };
            d = peek_char();
            if (isdigit(d)) {
                d = next_char();
                token.count = (int)d - 48;
                while (isdigit(peek_char())) {
                    d = next_char();
                    token.count *= 10;
                    token.count += ((int)d - 48);
                }
            }
            switch (d) {
            case '*':
                next_char();
                token.star = true;
                return token;
            case '_':
            case '!':
                next_char();
                if (d != 's' && d != 'S' && d != 'i' && d != 'I' && d != 'l' && d != 'L' && d != 'q' && d != 'Q' && d != 'j' && d != 'J') {
                    char buf[2] = { static_cast<char>(d), '\0' }; // FIXME: String::format needs some love :)
                    token.error = String::format("'{}' allowed only after types sSiIlLqQjJ", buf);
                }
                return token;
            default:
                return token;
            }
        }

        signed char next_char() {
            char c = peek_char();
            m_index++;
            return c;
        }

        signed char peek_char() {
            if (m_index >= m_directives->length())
                return 0;
            signed char c = (*m_directives)[m_index];
            while (isspace(c)) {
                if (m_index + 1 >= m_directives->length())
                    return 0;
                c = (*m_directives)[++m_index];
            }
            return c;
        }

        String *m_directives;
        size_t m_index { 0 };
    };

}

}
