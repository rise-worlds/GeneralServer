#pragma once

#include <eosio/chain/types.hpp>

namespace eosio { namespace chain {

   using allowlisted_intrinsics_type = shared_flat_multimap<uint64_t, shared_string>;

   // TODO: Improve performance by using std::string_view when we switch to C++17.

   bool is_intrinsic_allowlisted( const allowlisted_intrinsics_type& allowlisted_intrinsics, const std::string& name );

   void add_intrinsic_to_allowlist( allowlisted_intrinsics_type& allowlisted_intrinsics, const std::string& name );

   void remove_intrinsic_from_allowlist( allowlisted_intrinsics_type& allowlisted_intrinsics, const std::string& name );

   void reset_intrinsic_allowlist( allowlisted_intrinsics_type& allowlisted_intrinsics,
                                   const std::set<std::string>& s );

   std::set<std::string> convert_intrinsic_allowlist_to_set( const allowlisted_intrinsics_type& allowlisted_intrinsics );

} } // namespace eosio::chain
