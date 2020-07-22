#include <eosio/chain/name.hpp>
#include <fc/variant.hpp>
#include <boost/algorithm/string.hpp>
#include <fc/exception/exception.hpp>
#include <eosio/chain/exceptions.hpp>

namespace eosio::chain {

   void name::set( std::string_view str ) {
      const auto len = str.size();
      EOS_ASSERT(len <= 43, name_type_exception, "Name is longer than 43 characters (${name}) ", ("name", std::string(str)));
      value = string_to_uint256_t(str);
      EOS_ASSERT(to_string() == str, name_type_exception,
                 "Name not properly normalized (name: ${name}, normalized: ${normalized}) ",
                 ("name", std::string(str))("normalized", to_string()));
   }

   // keep in sync with name::to_string() in contract definition for name
   std::string name::to_string()const {
      static const char* charmap = ".-0123456789abcdefghijklmnopqrstuvwxyz_:<>[]{}()`~..............";
      std::string str(43, '.');

      fc::uint256_t tmp = value;
      tmp >>= 4;
      for( uint32_t i = 0; i < 42; ++i ) {
         char c = charmap[static_cast<uint8_t >(tmp.low_64_bits() & 0x3f)];
         str[i] = c;
         tmp >>= 6;
      }

      boost::algorithm::trim_right_if( str, []( char c ){ return c == '.'; } );
      return str;
      /*
      static const char* charmap = ".-0123456789abcdefghijklmnopqrstuvwxyz_:<>[]{}()`~..............";

      std::string str(43,'.');

      fc::uint256_t tmp = value;
      for( uint32_t i = 0; i <= 42; ++i ) {
         char c = charmap[tmp.low_64_bits() & (i == 0 ? 0x0f : 0x3f)];
         str[42-i] = c;
         tmp >>= (i == 0 ? 4 : 6);
      }

      boost::algorithm::trim_right_if( str, []( char c ){ return c == '.'; } );
      return str;
       */
   }

} // eosio::chain

namespace fc {
  void to_variant(const eosio::chain::name& c, fc::variant& v) { v = c.to_string(); }
  void from_variant(const fc::variant& v, eosio::chain::name& check) { check.set( v.get_string() ); }
} // fc
