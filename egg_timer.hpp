#ifndef EGG_TIMER_HPP
#define EGG_TIMER_HPP

#include <algorithm>
#include <atomic>
#include <functional>
#include <memory>
#include <tuple>


template<typename T>
class egg_timer
{
public:
    egg_timer() = delete;
    egg_timer &operator=(egg_timer const &) = default;
    egg_timer(egg_timer const&) = default;
    egg_timer &operator=(egg_timer &&) = default;
    egg_timer(egg_timer &&) = default;

    egg_timer(size_t bytes_between_updates, T &&progress_handler) :
        atomics{std::make_shared<atomic_pair>()},
        payload(std::move(progress_handler)),
        bytes_between_updates(bytes_between_updates) {

        using ProgressHandler_t = decltype (progress_handler);
        static_assert (std::is_invocable_v<ProgressHandler_t, size_t, size_t>, "egg_timer instantiated with inappropriate progress_handler argument.");
    }

    void operator()(size_t completed, size_t total) const noexcept {

    const size_t last_update = atomics->last_called_at;
    const size_t bytes_read_during_last_handler_call = atomics->bytes_read;
    const size_t total_bytes_read = (atomics->bytes_read = std::max(completed, bytes_read_during_last_handler_call));

    if(total_bytes_read-last_update > bytes_between_updates) {
        payload(completed, total);
        atomics->last_called_at = total_bytes_read;

    }

    }


private:
class atomic_pair {
public:
    atomic_pair() : bytes_read(0), last_called_at(0) { }
    std::atomic<size_t> bytes_read;
    std::atomic<size_t> last_called_at;
};

    mutable std::shared_ptr<atomic_pair> atomics;
    T payload;

protected:
    const size_t bytes_between_updates; // Every 1 MB

};


#endif // EGG_TIMER_HPP
