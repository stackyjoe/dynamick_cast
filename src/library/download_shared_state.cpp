#include <fmt/core.h>

//#include <QPersistentModelIndex>


#include "download_shared_state.hpp"


void download_shared_state::adopt_lock(std::unique_lock<std::mutex> &&lock) noexcept {
    lock_storage.emplace(std::move(lock));
}

std::unique_lock<std::mutex> download_shared_state::take_lock() {
    std::unique_lock<std::mutex> l = std::move(lock_storage.value());
    lock_storage = {};
    return l;
}


void download_shared_state::clear_lock() noexcept {
    lock_storage.reset();
}

size_t download_shared_state::get_bytes_completed() const noexcept {
    return completed_bytes;
}

size_t download_shared_state::get_bytes_total() const noexcept {
    return total_bytes.has_value()? *total_bytes : 0;
}

void download_shared_state::request_gui_update() const {
    gui_callback();
}

void download_shared_state::set_bytes_completed(size_t bytes) noexcept {
    completed_bytes = bytes;
}

void download_shared_state::set_bytes_total(size_t bytes) noexcept {
    if(!total_bytes.has_value())
        total_bytes.emplace(bytes);
}

void download_shared_state::set_gui_callback(std::function<void ()> &&callback) noexcept {
    gui_callback = callback;
}

std::optional<std::unique_lock<std::mutex>> download_shared_state::try_lock() const noexcept {
    std::unique_lock<std::mutex> lock(mtx, std::try_to_lock);

    if(lock.owns_lock())
        return {std::move(lock)};

    return std::nullopt;
}
