#ifndef STRING_FUNCTIONS_HPP
#define STRING_FUNCTIONS_HPP

#include <string>

#include <QString>

// This is a quick and dirty one someone wrote on stackoverflow
//          https://stackoverflow.com/a/46711735
// I'll implement something nicer in the future (unless I forget)

constexpr unsigned int hash(char const *s, int off = 0) {
    return !s[off] ? 5381 : (hash(s, off+1)*33) ^ static_cast<unsigned>(s[off]); // NOLINT
}

unsigned int hash(std::string const &s, int off=0);

constexpr unsigned int length(char const *s) {
    unsigned len = 0;
    while(*s != '\0') {
        ++len;
        ++s; // NOLINT
    }
    return len;
}

QString to_time(int seconds);

std::string sanitize(std::string const &raw);

#endif // STRING_FUNCTIONS_HPP
