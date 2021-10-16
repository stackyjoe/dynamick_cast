#include <exception>

#include <fmt/core.h>
#include <boost/beast/core/error.hpp>

inline void notify_and_ignore(std::exception const & e) {
    fmt::print("An exception has occurred: {}\n", e.what());
}

inline void notify_and_ignore(boost::beast::error_code const &ec) {
    fmt::print("An error has occurred: {}\n", ec.message());
}