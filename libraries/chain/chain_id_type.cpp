#include <potato/chain/chain_id_type.hpp>

namespace potato
{
    namespace chain
    {

    }
} // namespace potato

namespace fc
{
    class variant;
    void to_variant(const potato::chain::chain_id_type &cid, fc::variant &v)
    {
        to_variant(static_cast<const fc::sha256 &>(cid), v);
    }
    void from_variant(const fc::variant &v, potato::chain::chain_id_type &cid)
    {
        from_variant(v, static_cast<fc::sha256 &>(cid));
    }
} // namespace fc