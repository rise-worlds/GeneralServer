#pragma once
#include <string>
#include <fc/reflect/reflect.hpp>
#include <iosfwd>
#include <fc/uint256.hpp>

namespace eosio::chain {
   struct name;
   using uint256_t = fc::uint256_t;
   struct capi_name {
      uint64_t data[4];
   };
}
namespace fc {
  class variant;
  void to_variant(const eosio::chain::name& c, fc::variant& v);
  void from_variant(const fc::variant& v, eosio::chain::name& check);
} // fc

namespace eosio::chain {
   static const char encode_map[128] = {
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0, 46, 47,  0,  0,  1,  0,  0,
        2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 39,  0, 40,  0, 41,  0,
        0, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
       27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 42,  0, 43,  0, 38,
       48, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
       27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 44,  0, 45, 49,  0,
       };
   static const char decode_map[65] = ".-0123456789abcdefghijklmnopqrstuvwxyz_:<>[]{}()`~..............";

   static constexpr uint64_t char_to_symbol( char c ) {
      if( c == '~' )
        return (c - '~') + 49;
      if( c == '`' )
        return (c - '`') + 48;
      if( c == ')' )
        return (c - ')') + 47;
      if( c == '(' )
        return (c - '(') + 46;
      if( c == '}' )
        return (c - '}') + 45;
      if( c == '{' )
        return (c - '{') + 44;
      if( c == ']' )
        return (c - ']') + 43;
      if( c == '[' )
        return (c - '[') + 42;
      if( c == '>' )
        return (c - '>') + 41;
      if( c == '<' )
        return (c - '<') + 40;
      if( c == ':' )
        return (c - ':') + 39;
      if( c == '_' )
        return (c - '_') + 38;
      if( c >= 'A' && c <= 'Z' )
        return (c - 'A') + 12;
      if( c >= 'a' && c <= 'z' )
        return (c - 'a') + 12;
      if( c >= '0' && c <= '9' )
        return (c - '0') + 2;
      if( c == '-' )
        return (c - '-') + 1;
      return 0;
   }

   static constexpr uint256_t string_to_uint256_t( std::string_view str )
   {
      uint256_t name(0);
      int i = 0;
      for ( ; str[i] && i < 42; ++i) {
          // NOTE: char_to_symbol() returns char type, and without this explicit
          // expansion to uint64 type, the compilation fails at the point of usage
          // of string_to_name(), where the usage requires constant (compile time) expression.
          //name |= (char_to_symbol(str[i]) & 0x3f) << (64 - 6 * (i + 1));
           uint256_t c = (char_to_symbol(str[i]) & 0x3f);
           c <<= (4 + (6 * i));
           name |= c;
       }

       // The for-loop encoded up to 60 high bits into uint64 'name' variable,
       // if (strlen(str) > 12) then encode str[12] into the low (remaining)
       // 4 bits of 'name'
       //if (i == 10)
       //    name |= char_to_symbol(str[10]) & 0x0F;
      return name;
   }

   /// Immutable except for fc::from_variant.
   struct name {
      union {
         //std::array<uint8_t, 32> bytes;
         std::array<char, 32> bytes = { 0 };
         //std::array<uint32_t, 8> dwords;
         std::array<uint64_t, 4> qwords;
         //std::array<__uint128_t, 2> qtwords;
         uint256_t value;
      };

   private:

      friend struct fc::reflector<name>;
      friend void fc::from_variant(const fc::variant& v, eosio::chain::name& check);

      void set( std::string_view str );

   public:
      constexpr bool empty()const { return uint256_t(0) == value; }
      constexpr bool good()const  { return !empty();   }

      explicit name( const char* str ) { set( str ); }
      explicit name( const std::string& str ) { set( str ); }
      explicit name( std::string_view str ) { set( str ); }
      constexpr explicit name( uint64_t v ) : value(v) {}
      constexpr explicit name( uint256_t& v ) : value(v) {}
      constexpr explicit name( const uint256_t& v ) : value(v) {}
      constexpr explicit name( const capi_name& v ) {
         qwords[0] = v.data[0];
         qwords[1] = v.data[1];
         qwords[2] = v.data[2];
         qwords[3] = v.data[3];
      }
      template<typename T>
      name( T v ):value(v){}
      constexpr name():value(){}
      constexpr name(name& v) : value(v.value) {}
      constexpr name(const name& v) : value(v.value) {}

      std::string to_string()const;
      constexpr uint256_t to_uint256_t()const { return value; }
      // name& operator=( const uint256_t& v ) {
      //    value = v;
      //    return *this;
      // }
      friend std::ostream& operator << ( std::ostream& out, const name& n ) {
         return out << n.to_string();
      }

      friend bool operator < ( const name& a, const name& b ) { return a.value < b.value; }
      friend bool operator > ( const name& a, const name& b ) { return a.value > b.value; }
      friend bool operator <= ( const name& a, const name& b ) { return a.value <= b.value; }
      friend bool operator >= ( const name& a, const name& b ) { return a.value >= b.value; }
      friend bool operator == ( const name& a, const name& b ) { return a.value == b.value; }
      friend bool operator != ( const name& a, const name& b ) { return a.value != b.value; }

      friend constexpr bool operator == ( const name& a, const uint256_t& b ) { return a.value == b; }
      friend constexpr bool operator != ( const name& a, const uint256_t& b ) { return a.value != b; }
      friend constexpr bool operator == ( const uint256_t& a, const name& b ) { return a == b.value; }
      friend constexpr bool operator != ( const uint256_t& a, const name& b ) { return a != b.value; }

      constexpr explicit operator bool()const { return value != 0; }
      constexpr explicit operator uint256_t()const       { return static_cast<uint256_t>(value); }
   };

   // Each char of the string is encoded into 6-bit chunk and left-shifted
   // to its 6-bit slot starting with the highest slot for the first char.
   // The 43th char, if str is long enough, is encoded into 4-bit chunk
   // and placed in the lowest 4 bits. 256 = 42 * 6 + 4
   static constexpr name string_to_name( std::string_view str )
   {
      return name( string_to_uint256_t( str ) );
   }

#define N(X) eosio::chain::string_to_name(#X)

} // eosio::chain

namespace std {
   template<> struct hash<eosio::chain::name> : private hash<std::string> {
      typedef eosio::chain::name argument_type;
      typedef typename hash<std::string>::result_type result_type;
      result_type operator()(const argument_type& name) const noexcept
      {
         return hash<std::string>::operator()(name.to_string());
      }
   };
};

FC_REFLECT( eosio::chain::name, (value) )
