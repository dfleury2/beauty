#pragma once

#include <iostream>
#include <fstream>
#include <iterator>
#include <filesystem>

namespace fs = std::filesystem;

//------------------------------------------------------------------------------
inline
std::string
read_file_content(const fs::path& filename, bool binary = false)
{
    std::cout << "Reading file: " << filename << std::endl;

    std::ifstream file{filename, (binary ? std::ios_base::binary : std::ios_base::in)};
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}
