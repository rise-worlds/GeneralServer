#include <eosio/chain/protocol_state_object.hpp>

namespace eosio { namespace chain {

   namespace detail {

      snapshot_protocol_state_object
      snapshot_row_traits<protocol_state_object>::to_snapshot_row( const protocol_state_object& value,
                                                                   const chainbase::database& db )
      {
         snapshot_protocol_state_object res;

         res.whitelisted_intrinsics = convert_intrinsic_whitelist_to_set( value.whitelisted_intrinsics );

         res.num_supported_key_types = value.num_supported_key_types;

         return res;
      }

      void
      snapshot_row_traits<protocol_state_object>::from_snapshot_row( snapshot_protocol_state_object&& row,
                                                                     protocol_state_object& value,
                                                                     chainbase::database& db )
      {
         reset_intrinsic_whitelist( value.whitelisted_intrinsics, row.whitelisted_intrinsics );

         value.num_supported_key_types = row.num_supported_key_types;
      }

   }

}}
