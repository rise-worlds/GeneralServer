#pragma once
#include <eosio/chain/block_header.hpp>
#include <eosio/chain/incremental_merkle.hpp>
#include <eosio/chain/chain_snapshot.hpp>
#include <future>

namespace eosio { namespace chain {

using signer_callback_type = std::function<std::vector<signature_type>(const digest_type&)>;

struct block_header_state;

namespace detail {
   struct block_header_state_common {
      uint32_t                          block_num = 0; //最近的block的高度
      uint32_t                          dpos_proposed_irreversible_blocknum = 0; //最新的被提出dpos不可逆的区块高度，生产这个块的节点提议该值的块号为不可逆块，这个值在第一阶段计算出来。在 setconfirmed 函数中计算出来
      uint32_t                          dpos_irreversible_blocknum = 0; //最新的dpos不可逆区块高度
      producer_authority_schedule       active_schedule;
      incremental_merkle                blockroot_merkle;
      flat_map<account_name,uint32_t>   producer_to_last_produced;      //该BP认为的异步BFT共识算法第一阶段(Pre-Commit)的结果，记录的每个生产者最后一次生产的块号。每个生产者在它生产块时候通过它就可以计算出它需要确认多少个块。把计算出来的值放到块头上，广播该块时候其它收到该块的节点就可以知道这个生产者确认了多少块。
      flat_map<account_name,uint32_t>   producer_to_last_implied_irb;   //该BP认为的异步BFT共识算法第二阶段(Commit)的结果，主要用来计算不可逆块号。它是一个map结构，记录的是每个生产节点它的建议块号。
      block_signing_authority           valid_block_signing_authority;
      vector<uint8_t>                   confirm_count;   //记录的是每个块到目前为止还需要多少个生产者来确认它，当某个元素值变为0后，说明该元素所对应的块应该是生产该块的节点的建议不可逆块号。然后把块号赋给 dpos_proposed_irreversible_blocknum.按顺序存储了所有尚未不可逆的区块的确认次数。初始值为2/3*bp_count+1，每确认一次减1，为0则表示有足够的BP确认，会从该数组删除。
      bool                              enable_standby_schedule = false;
      optional<block_num_type>          standby_schedule_block_num;
   };

   struct schedule_info {
      uint32_t                          schedule_lib_num = 0; /// last irr block num
      digest_type                       schedule_hash;
      producer_authority_schedule       schedule;
   };
}

struct pending_block_header_state : public detail::block_header_state_common {
   detail::schedule_info                prev_pending_schedule;
   bool                                 was_pending_promoted = false;
   block_id_type                        previous;
   account_name                         producer;
   block_timestamp_type                 timestamp;
   uint32_t                             active_schedule_version = 0;
   uint16_t                             confirmed = 1;

   signed_block_header make_block_header( const checksum256_type& transaction_mroot,
                                          const checksum256_type& action_mroot,
                                          const optional<producer_authority_schedule>& new_producers,
                                          bool enable_standby_schedule,
                                          optional<block_num_type> standby_schedule_block_num )const;

   block_header_state  finish_next( const signed_block_header& h,
                                    vector<signature_type>&& additional_signatures,
                                    bool skip_validate_signee = false )&&;

   block_header_state  finish_next( signed_block_header& h,
                                    const signer_callback_type& signer )&&;

protected:
   block_header_state  _finish_next( const signed_block_header& h )&&;
};

/**
 *  @struct block_header_state
 *  @brief defines the minimum state necessary to validate transaction headers
 */
struct block_header_state : public detail::block_header_state_common {
   block_id_type                        id;                    //最近的block_id
   signed_block_header                  header;                //最近的block header;
   detail::schedule_info                pending_schedule;
   vector<signature_type>               additional_signatures;

   /// this data is redundant with the data stored in header, but it acts as a cache that avoids
   /// duplication of work
   flat_multimap<uint16_t, block_header_extension> header_exts;

   block_header_state() = default;

   explicit block_header_state( detail::block_header_state_common&& base )
   :detail::block_header_state_common( std::move(base) )
   {}

   pending_block_header_state  next( block_timestamp_type when,
                                    uint16_t num_prev_blocks_to_confirm )const;

   block_header_state   next( const signed_block_header& h,
                              vector<signature_type>&& additional_signatures,
                              bool skip_validate_signee = false )const;

   bool                 has_pending_producers()const { return pending_schedule.schedule.producers.size(); }
   uint32_t             calc_dpos_last_irreversible( account_name producer_of_next_block )const;

   producer_authority     get_scheduled_producer( block_timestamp_type t )const;
   const block_id_type&   prev()const { return header.previous; }
   digest_type            sig_digest()const;
   void                   sign( const signer_callback_type& signer );
   void                   verify_signee()const;
};

using block_header_state_ptr = std::shared_ptr<block_header_state>;

} } /// namespace eosio::chain

FC_REFLECT( eosio::chain::detail::block_header_state_common,
            (block_num)
            (dpos_proposed_irreversible_blocknum)
            (dpos_irreversible_blocknum)
            (active_schedule)
            (blockroot_merkle)
            (producer_to_last_produced)
            (producer_to_last_implied_irb)
            (valid_block_signing_authority)
            (confirm_count)
            (enable_standby_schedule)
            (standby_schedule_block_num)
)

FC_REFLECT( eosio::chain::detail::schedule_info,
            (schedule_lib_num)
            (schedule_hash)
            (schedule)
)

FC_REFLECT_DERIVED(  eosio::chain::block_header_state, (eosio::chain::detail::block_header_state_common),
                     (id)
                     (header)
                     (pending_schedule)
                     (additional_signatures)
)
