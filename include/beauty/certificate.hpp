#pragma once

#include <string>

namespace beauty
{
// --------------------------------------------------------------------------
struct certificates {
    std::string certificat_chain;
    std::string private_key;
    std::string temporary_dh;
    std::string password;

    bool is_valid() const { return certificat_chain.size(); }
};

}
