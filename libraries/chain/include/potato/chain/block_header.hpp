#pragma once
#include <potato/chain/block_timestamp.hpp>
#include <potato/chain/types.hpp>

namespace potato::chain
{
struct block_header
{
    block_timestamp_type    timestamp;
    account_name            producer;

    uint16_t                confirmed = 1;
    block_id_type           previous;

    uint32_t                schedule_version = 0;

    block_header() = default;

    digest_type     digest() const;
    block_id_type   id() const;
    uint32_t        block_num() const { return num_from_id(previous) + 1; }
    static uint32_t num_from_id(const block_id_type& id);
};

struct signed_block_header : public block_header
{
    signature_type  producer_signature;
};
}
FC_REFLECT(potato::chain::block_header, (timestamp)(producer)(confirmed)(schedule_version))
FC_REFLECT_DERIVED(potato::chain::signed_block_header, (potato::chain::block_header), (producer_signature))
