#ifndef DOWNLOAD_SHARED_STATE_HPP
#define DOWNLOAD_SHARED_STATE_HPP

#include <atomic>
#include <functional>
#include <mutex>
#include <optional>

class QModelIndex;

class download_shared_state
{
public:
    download_shared_state()
    : mtx(),
      lock_storage(std::nullopt),
      completed_bytes(0),
      bytes_since_last_gui_update(0),
      total_bytes(std::nullopt)
    {

    }
    download_shared_state(download_shared_state const &) = delete;
    download_shared_state &operator=(download_shared_state const &) = delete;

    download_shared_state(download_shared_state &&) = delete;
    download_shared_state &operator=(download_shared_state &&) = delete;

    void adopt_lock(std::unique_lock<std::mutex> &&lock) noexcept;
    [[nodiscard]] std::unique_lock<std::mutex> take_lock();
    void clear_lock() noexcept;
    size_t get_bytes_completed() const noexcept;
    size_t get_bytes_total() const noexcept;
    void request_gui_update() const;
    void set_bytes_completed(size_t bytes) noexcept;
    void set_bytes_total(size_t bytes) noexcept;
    void set_gui_callback(std::function<void()> &&callback) noexcept;
    std::optional<std::unique_lock<std::mutex>> try_lock() const noexcept;

private:
    mutable std::mutex mtx;
    mutable std::optional<std::unique_lock<std::mutex>> lock_storage;
    std::atomic<size_t> completed_bytes;
    std::atomic<size_t> bytes_since_last_gui_update;
    std::optional<size_t> total_bytes;
    std::function<void()> gui_callback;
};

#endif // DOWNLOAD_SHARED_STATE_HPP
