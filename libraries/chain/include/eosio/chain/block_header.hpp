#pragma once
#include <eosio/chain/block_timestamp.hpp>
#include <eosio/chain/producer_schedule.hpp>

#include <type_traits>

namespace eosio { namespace chain {

   namespace detail {
      template<typename... Ts>
      struct block_header_extension_types {
         using block_header_extension_t = fc::static_variant< Ts... >;
         using decompose_t = decompose< Ts... >;
      };
   }

   using block_header_extension_types = detail::block_header_extension_types<
      producer_schedule_change_extension
   >;

   using block_header_extension = block_header_extension_types::block_header_extension_t;

   struct block_header
   {
      block_timestamp_type             timestamp;  //生产该块的时间
      account_name                     producer;   //生产者的账号

      /**
       *  By signing this block this producer is confirming blocks [block_num() - confirmed, blocknum())
       *  as being the best blocks for that range and that he has not signed any other
       *  statements that would contradict.
       *
       *  No producer should sign a block with overlapping ranges or it is proof of byzantine
       *  behavior. When producing a block a producer is always confirming at least the block he
       *  is building off of.  A producer cannot confirm "this" block, only prior blocks.
       */
      uint16_t                         confirmed = 1;

      block_id_type                    previous; //前一块的HASH

      checksum256_type                 transaction_mroot; /// mroot of cycles_summary
      checksum256_type                 action_mroot; /// mroot of all delivered action receipts

      uint32_t                          schedule_version = 0;
      extensions_type                   header_extensions; //新生产者

      block_header() = default;

      digest_type       digest()const;
      block_id_type     id() const;
      uint32_t          block_num() const { return num_from_id(previous) + 1; }
      static uint32_t   num_from_id(const block_id_type& id);

      flat_multimap<uint16_t, block_header_extension> validate_and_extract_header_extensions()const;
   };


   struct signed_block_header : public block_header
   {
      signature_type    producer_signature; // 生产者签名
   };

} } /// namespace eosio::chain

FC_REFLECT(eosio::chain::block_header,
           (timestamp)(producer)(confirmed)(previous)
           (transaction_mroot)(action_mroot)
           (schedule_version)
           (header_extensions))

FC_REFLECT_DERIVED(eosio::chain::signed_block_header, (eosio::chain::block_header), (producer_signature))
