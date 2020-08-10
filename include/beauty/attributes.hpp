#pragma once

#include <string>
#include <unordered_map>

namespace beauty
{
// --------------------------------------------------------------------------
class attributes
{
public:
    attributes() = default;
    attributes(const std::string& str, char sep = '&');

    void insert(std::string key, std::string value);

    const std::string& operator[](const std::string& key) const;

private:
    std::unordered_map<std::string, std::string> _attributes;

    inline static const std::string EMPTY;
};

}
