#pragma once

#include <string>

namespace beauty
{
// --------------------------------------------------------------------------
class attribute
{
public:
    attribute() = default;
    explicit attribute(std::string str) :
            _value(std::move(str))
    {}

    std::string as_string(const char* default_value = "") const noexcept
    { return as<std::string>(default_value); }

    int as_integer(int default_value = 0) const
    { return as<int>(default_value); }

    double as_double(double default_value = 0.0) const
    { return as<double>(default_value); }

    bool as_boolean(bool default_value = false) const
    {
        return as<bool>(default_value);
    }

    bool operator==(const char* other) const noexcept { return _value == other; }
    bool operator==(const std::string& other) const noexcept { return _value == other; }

private:
    template<typename Type>
    Type as(const Type& default_value = {}) const {
        if (_value.empty()) {
            return default_value;
        }

        if constexpr (std::is_same_v<Type, bool>) {
            return (_value == "1" || _value == "true" || _value == "yes");
        }
         else if constexpr (std::is_floating_point_v<Type>) {
            return std::stod(_value);
        }
        else if constexpr (std::is_integral_v<Type>) {
            return std::stoi(_value);
        }
        else {
            return _value;
        }
    }

private:
    std::string _value;
};

}
