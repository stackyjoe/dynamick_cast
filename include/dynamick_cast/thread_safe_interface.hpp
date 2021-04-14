#ifndef THREAD_SAFE_INTERFACE_HPP
#define THREAD_SAFE_INTERFACE_HPP

#include <optional>
#include <memory>
#include <mutex>
#include <type_traits>

/// \brief This is a general purpose thread safe interface class.
///
template<typename Facade>
class  thread_safe_interface {
private:

    std::unique_ptr<Facade> implementation;

    
    thread_safe_interface(std::unique_ptr<Facade> implementation)
        : implementation(std::move(implementation)) {
    }

    Facade & get_ref() noexcept {
        return *implementation;
    }

    Facade const & get_const_ref() const noexcept {
        return *implementation;
    }

    Facade volatile & get_vol_ref() noexcept {
        return *implementation;
    }

public:
    bool is_null() const noexcept {
        return implementation == nullptr;
    }

    bool is_not_null() const noexcept {
        return implementation;
    }

    thread_safe_interface() = default;
    // Copy constructor and copy assignment are implicitly forbidden by virtue of owning a unique_ptr and mutex.
    thread_safe_interface(thread_safe_interface &&) = default;
    thread_safe_interface &operator=(thread_safe_interface &&) = default;

    thread_safe_interface(thread_safe_interface const &) = delete;
    thread_safe_interface &operator=(thread_safe_interface const &) = delete;

    ~thread_safe_interface() = default;


    template<typename Implementation, typename ... ConstructorArgs>
    static thread_safe_interface make(ConstructorArgs && ... constructor_args) {
        static_assert (std::is_base_of<Facade, Implementation>::value,
                       "thread_safe_interface::make must be called with an implementation of the facade class.");
        //std::unique_ptr<Facade> impl = std::make_unique<Implementation>(std::forward<ConstructorArgs>(constructor_args) ...);
        return thread_safe_interface(std::unique_ptr<Facade>(std::make_unique<Implementation>(std::forward<ConstructorArgs>(constructor_args) ...)));
    }



    // Trick from I learned from a Herb Sutter talk.
    template<typename operation>
    auto perform(operation o) -> decltype (o(get_ref())) {
        static_assert (std::is_invocable<decltype(o), decltype(get_ref())>::value,
                       "thread_safe_interface::perform should be called on lambda invocable with a Facade & argument.");
        std::scoped_lock access(get_ref().access_lock);
        return o(get_ref());
    }

    template<typename operation>
    auto try_perform(operation o) -> std::optional<decltype(o(get_ref()))> {
        static_assert (std::is_invocable<decltype(o), decltype(get_ref())>::value,
                       "thread_safe_interface::try_perform should be called on lambda invocable with a Facade & argument.");

        if(std::unique_lock access(get_ref().access_lock, std::try_to_lock); not access.owns_lock())
            return std::nullopt;
        return o(get_ref());
    }

    template<typename operation>
    auto perform_unsynchronized(operation o) const -> decltype(o(get_const_ref())) {
        static_assert (std::is_invocable<decltype(o), decltype(get_const_ref())>::value,
                       "thread_safe_interface::perform_unsynchronized should be called on lambda invocable with a Facade const & argument.");
        return o(get_const_ref());
    }
};

#endif // THREAD_SAFE_INTERFACE_HPP
