#include <fmt/core.h>

//#include <QPersistentModelIndex>


#include "download_shared_state.hpp"


void download_shared_state::adopt_lock(std::unique_lock<std::mutex> &&lock) noexcept {
    lock_storage.emplace(std::move(lock));
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

/*
std::optional<QModelIndex> download_shared_state::get_index() const noexcept {
    if(index_storage.has_value()) {
        if(not index_storage->isValid()) {
            index_storage.reset();
            return std::nullopt;
        }
        return *index_storage;
    }

    return std::nullopt;
}*/

void download_shared_state::request_gui_update() const {
    gui_callback();
}

void download_shared_state::set_bytes_completed(size_t bytes) noexcept {
    completed_bytes = bytes;
}

void download_shared_state::set_bytes_total(size_t bytes) noexcept {
    if(not total_bytes.has_value())
        total_bytes.emplace(bytes);
}

void download_shared_state::set_gui_callback(std::function<void ()> &&callback) noexcept{
    gui_callback = callback;
}

/*
void download_shared_state::set_index(QModelIndex index) {
    index_storage.emplace(index);
}
*/

std::optional<std::unique_lock<std::mutex>> download_shared_state::try_lock() const noexcept {

    std::unique_lock<std::mutex> lock(mtx, std::try_to_lock);


    if(lock.owns_lock())
        return {std::move(lock)};

    return std::nullopt;
}
