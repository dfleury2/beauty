#pragma once

#include <beauty/attribute.hpp>
#include <beauty/export.hpp>

#include <string>
#include <unordered_map>

namespace beauty
{
// --------------------------------------------------------------------------
class BEAUTY_EXPORT attributes
{
public:
    using attribute_storage = std::unordered_map<std::string, attribute>;

    attributes() = default;
    explicit attributes(const std::string& str, char sep = '&');

    void insert(std::string key, std::string value);

    attribute_storage::const_iterator find(const std::string& key) const {
        return _attributes.find(key);
    }

    attribute_storage::const_iterator begin() const { return _attributes.begin(); }
    attribute_storage::const_iterator end() const { return _attributes.end(); }

    const attribute& operator[](const std::string& key) const;

private:
    attribute_storage _attributes;

    inline static const attribute EMPTY;
};

}
