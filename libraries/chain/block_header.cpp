#include <potato/chain/block_header.hpp>
#include <fc/io/raw.hpp>
#include <fc/bitutil.hpp>
#include <algorithm>

namespace potato::chain
{
    digest_type block_header::digest() const 
    {
        return digest_type::hash(*this);
    }

    uint32_t block_header::num_from_id(const block_id_type& id)
    {
        return fc::endian_reverse_u32(id._hash[0]);
    }

    block_id_type block_header::id() const 
    {
        block_id_type result = digest();
        result._hash[0] &= 0xFFFFFFFF00000000ull;
        result._hash[0] += fc::endian_reverse_u32(block_num());
        return result;
    }
}