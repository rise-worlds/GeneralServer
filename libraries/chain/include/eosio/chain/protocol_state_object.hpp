#pragma once

#include <eosio/chain/types.hpp>
#include <eosio/chain/snapshot.hpp>
#include <eosio/chain/allowlisted_intrinsics.hpp>
#include <chainbase/chainbase.hpp>
#include "multi_index_includes.hpp"

namespace eosio { namespace chain {

   /**
    * @class protocol_state_object
    * @brief Maintains global state information about consensus protocol rules
    * @ingroup object
    * @ingroup implementation
    */
   class protocol_state_object : public chainbase::object<protocol_state_object_type, protocol_state_object>
   {
      OBJECT_CTOR(protocol_state_object, (allowlisted_intrinsics))

   public:
      id_type                                    id;
      allowlisted_intrinsics_type                allowlisted_intrinsics;
      uint32_t                                   num_supported_key_types = 0;
   };

   using protocol_state_multi_index = chainbase::shared_multi_index_container<
      protocol_state_object,
      indexed_by<
         ordered_unique<tag<by_id>,
            BOOST_MULTI_INDEX_MEMBER(protocol_state_object, protocol_state_object::id_type, id)
         >
      >
   >;

   struct snapshot_protocol_state_object {
      std::set<std::string>                                     allowlisted_intrinsics;
      uint32_t                                                  num_supported_key_types = 0;
   };

   namespace detail {
      template<>
      struct snapshot_row_traits<protocol_state_object> {
         using value_type = protocol_state_object;
         using snapshot_type = snapshot_protocol_state_object;

         static snapshot_protocol_state_object to_snapshot_row( const protocol_state_object& value,
                                                                const chainbase::database& db );

         static void from_snapshot_row( snapshot_protocol_state_object&& row,
                                        protocol_state_object& value,
                                        chainbase::database& db );
      };
   }

}}

CHAINBASE_SET_INDEX_TYPE(eosio::chain::protocol_state_object, eosio::chain::protocol_state_multi_index)

FC_REFLECT(eosio::chain::protocol_state_object,
            (allowlisted_intrinsics)(num_supported_key_types)
          )

FC_REFLECT(eosio::chain::snapshot_protocol_state_object,
            (allowlisted_intrinsics)(num_supported_key_types)
          )
