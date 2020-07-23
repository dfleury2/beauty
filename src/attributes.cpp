#include <beauty/attributes.hpp>

#include "utils.hpp"

namespace beauty
{
// --------------------------------------------------------------------------
attributes::attributes(const std::string& str, char sep)
{
    for(auto&& a : split(str, sep)) {
        auto kv = split(a, '=');
        if (kv.size() == 2) {
            _attributes.emplace(std::string(kv[0]), std::string(kv[1]));
        }
    }
}

// --------------------------------------------------------------------------
void
attributes::insert(std::string key, std::string value)
{
    _attributes.emplace(std::move(key), std::move(value));
}

// --------------------------------------------------------------------------
const std::string&
attributes::operator[](const std::string& key) const
{
    static const std::string EMPTY;

    auto found_key = _attributes.find(key);
    if (found_key != _attributes.end()) {
        return found_key->second;
    }

    return EMPTY;
}

}
