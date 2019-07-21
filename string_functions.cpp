#include "string_functions.hpp"

unsigned int hash(const std::string s, int off) {
    return hash(s.c_str(), off);
}

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

QString to_time(int seconds) {
    int sec, min, hours;
    sec = seconds % 60;
    seconds /= 60;
    min = seconds % 60;
    seconds /= 60;
    hours = seconds;

    QString s = sec >= 10 ? QString::number(sec) : "0"+QString::number(sec);
    QString m = min >= 10 ? QString::number(min) : "0"+QString::number(min);
    QString h = QString::number(hours);

    return h + ":" + m + ":" + s;
}
