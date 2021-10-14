#include "fmt/core.h"

#include "macros.hpp"

template<class ... Args>
inline void debug_print(Args &&... args) {
    #ifndef NDEBUG
     fmt::print(FWD(args)...);
    #endif
}