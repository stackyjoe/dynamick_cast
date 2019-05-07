#include "string_functions.hpp"

std::string sanitize(std::string raw) {
    std::string clean;
    clean.reserve(raw.size()*2);
    for(auto ch : raw) {
        switch(ch) {
        case '\"':
            clean += "\\\"";
            break;
        default:
            clean += ch;
            break;
        }
    }

    return clean;
}
