#pragma once
#include <potato/chain/types.hpp>
#include <potato/chain/chain_id_type.hpp>

#include <fc/crypto/sha256.hpp>

#include <string>

namespace potato
{
    namespace chain
    {
        struct genesis_state
        {
            genesis_state();

            static const string potato_root_key;

            time_point initial_timestamp;
            public_key_type initial_key;

            /**
             * Get the chain_id corresponding to this genesis state.
             *
             * This is the SHA256 serialization of the genesis_state.
             */
            chain_id_type compute_chain_id() const;

            friend inline bool operator==(const genesis_state &lhs, const genesis_state &rhs)
            {
                return std::tie(lhs.initial_timestamp, lhs.initial_key) == std::tie(rhs.initial_timestamp, rhs.initial_key);
            };

            friend inline bool operator!=(const genesis_state &lhs, const genesis_state &rhs) { return !(lhs == rhs); }
        };

    } // namespace chain
} // namespace potato

FC_REFLECT(potato::chain::genesis_state,
           (initial_timestamp)(initial_key))
