#ifndef DOWNLOAD_SHARED_STATE_HPP
#define DOWNLOAD_SHARED_STATE_HPP

#include <atomic>
#include <functional>
#include <mutex>
#include <optional>

class download_shared_state
{
public:
    struct notify_on_destruct {
    private:
        mutable std::atomic<bool>* atomic_to_change;
    public:
        bool should_start_downloading;

        download_shared_state::notify_on_destruct(download_shared_state* that)
            : atomic_to_change(&(that->currently_downloading)),
            should_start_downloading(!atomic_to_change->exchange(true)) { }

        notify_on_destruct& operator=(notify_on_destruct const &that) noexcept {
            atomic_to_change = std::exchange(that.atomic_to_change, nullptr);
        }

        notify_on_destruct(notify_on_destruct const &that) noexcept {
            atomic_to_change = std::exchange(that.atomic_to_change, nullptr);
        }

        notify_on_destruct& operator=(notify_on_destruct&& that) noexcept {
            atomic_to_change = std::exchange(that.atomic_to_change, nullptr);
        }

        notify_on_destruct(notify_on_destruct&& that) noexcept {
            atomic_to_change = std::exchange(that.atomic_to_change, nullptr);
        }

        ~notify_on_destruct() {
            if(atomic_to_change)
                atomic_to_change->store(false);
        }
    };

    download_shared_state()
    : currently_downloading(false),
      completed_bytes(0),
      bytes_since_last_gui_update(0),
      total_bytes(std::nullopt)
    {

    }
    download_shared_state(download_shared_state const &) = delete;
    download_shared_state &operator=(download_shared_state const &) = delete;

    download_shared_state(download_shared_state &&) = delete;
    download_shared_state &operator=(download_shared_state &&) = delete;

    notify_on_destruct should_start_downloading() noexcept;
    bool is_downloading() const noexcept;
    size_t get_bytes_completed() const noexcept;
    size_t get_bytes_total() const noexcept;
    void request_gui_update() const;
    void set_bytes_completed(size_t bytes) noexcept;
    void set_bytes_total(size_t bytes) noexcept;
    void set_gui_callback(std::function<void()> &&callback) noexcept;

private:

    std::atomic<bool> currently_downloading;
    std::atomic<size_t> completed_bytes;
    std::atomic<size_t> bytes_since_last_gui_update;
    std::optional<size_t> total_bytes;
    std::function<void()> gui_callback;
};

#endif // DOWNLOAD_SHARED_STATE_HPP
