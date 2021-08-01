/*
    sha1.h - header of

    ============
    SHA-1 in C++
    ============

    100% Public Domain.

    Original C Code
        -- Steve Reid <steve@edmweb.com>
    Small changes to fit into bglibs
        -- Bruce Guenter <bruce@untroubled.org>
    Translation to simpler C++ Code
        -- Volker Grabsch <vog@notjusthosting.com>
*/

#include <iostream>
#include <array>
#include <cstdint>
#include <sstream>

namespace beauty {

using digest_type = std::array<uint8_t, 20>;

class SHA1
{
public:
    SHA1();

    void update(const std::string& s);
    void update(std::istream& is);

    std::string final();
    digest_type digest();

    static std::string from_file(const std::string& filename);

private:
    // number of 32bit integers per SHA1 digest
    static const std::size_t digest_ints = 5;
    // number of 32bit integers per SHA1 block */
    static const std::size_t block_ints = 16;
    static const std::size_t block_bytes = block_ints * 4;

    using block_type = uint32_t[block_bytes];
    using int_block_type = uint32_t[block_ints];

    void reset();
    void transform(block_type block);
    void compute_digest();

    static void buffer_to_block(const std::string& buffer, block_type block);
    static void read(std::istream& is, std::string& s, std::size_t max);

    uint32_t digest_[digest_ints];
    std::string buffer_;
    uint64_t transforms_;
};

}
