#pragma once

#include <beauty/attribute.hpp>

#include <string>
#include <unordered_map>

namespace beauty
{
// --------------------------------------------------------------------------
class attributes
{
public:
    attributes() = default;
    explicit attributes(const std::string& str, char sep = '&');

    void insert(std::string key, std::string value);

    const attribute& operator[](const std::string& key) const;

private:
    std::unordered_map<std::string, attribute> _attributes;

    inline static const attribute EMPTY;
};

}
