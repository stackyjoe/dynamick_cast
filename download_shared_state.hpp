#ifndef DOWNLOAD_SHARED_STATE_HPP
#define DOWNLOAD_SHARED_STATE_HPP

#include <atomic>
#include <mutex>
#include <optional>

#include <QPersistentModelIndex>

class download_shared_state
{
public:
    download_shared_state();

    download_shared_state(download_shared_state const &) = delete;
    download_shared_state &operator=(download_shared_state const &) = delete;

    download_shared_state(download_shared_state &&) = delete;
    download_shared_state &operator=(download_shared_state &&) = delete;

    void adopt_lock(std::unique_lock<std::mutex> &&lock) noexcept;
    void clear_lock() noexcept;
    size_t get_bytes_completed() const noexcept;
    size_t get_bytes_total() const noexcept;
    std::optional<QModelIndex> get_index() const noexcept;
    void request_gui_update() const;
    void set_bytes_completed(size_t bytes) noexcept;
    void set_bytes_total(size_t bytes) noexcept;
    void set_gui_callback(std::function<void(QModelIndex&)> &&callback) noexcept;
    void set_index(QModelIndex index);
    std::optional<std::unique_lock<std::mutex>> try_lock() const noexcept;

protected:
    mutable std::mutex mtx;
    mutable std::optional<std::unique_lock<std::mutex>> lock_storage;
    std::atomic<size_t> completed_bytes;
    std::atomic<size_t> bytes_since_last_gui_update;
    std::optional<size_t> total_bytes;
    std::function<void(QModelIndex&)> gui_callback;
    mutable std::optional<QModelIndex> index_storage;
};

#endif // DOWNLOAD_SHARED_STATE_HPP
