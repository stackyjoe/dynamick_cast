#include <fmt/core.h>

#include "download_shared_state.hpp"

download_shared_state::notify_on_destruct download_shared_state::should_start_downloading() noexcept {
    return notify_on_destruct(this);
}

bool download_shared_state::is_downloading() const noexcept {
    return currently_downloading.load();
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
