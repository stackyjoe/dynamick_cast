#include <exception>

#include <fmt/core.h>

inline void notify_and_ignore(std::exception const & e) {
    fmt::print("An exception has occurred: {}\n", e.what());
}