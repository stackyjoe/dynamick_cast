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

std::string to_time(int seconds) {
    int sec, min, hours;
    sec = seconds % 60;
    seconds /= 60;
    min = seconds % 60;
    seconds /= 60;
    hours = seconds;

    std::string s = sec >= 10 ? std::to_string(sec) : "0"+std::to_string(sec);
    std::string m = min >= 10 ? std::to_string(min) : "0"+std::to_string(min);
    std::string h = std::to_string(hours);

    return h + ":" + m + ":" + s;
}
