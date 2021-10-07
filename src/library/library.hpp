#include <map>

#include "podcast.hpp"


struct library {
private:
    std::map<std::string, podcast> channels;
    static constexpr int version = 0;

public:

    using iterator = std::map<std::string,podcast>::iterator;
    using const_iterator = std::map<std::string,podcast>::const_iterator;

    iterator begin() noexcept;
    const_iterator begin() const noexcept;
    
    iterator end() noexcept;
    const_iterator end() const noexcept;

    size_t size() const noexcept;
    bool empty() const noexcept;

    podcast * find(std::string const &name) noexcept;
    podcast const * find(std::string const &name) const noexcept;

    void erase(std::string const &name);

    std::pair<iterator, bool> insert_or_assign(std::string const &key, podcast && value);

    void serialize_into(std::ofstream &save_file);
    void fill_from_xml(std::ifstream &save_file);

};