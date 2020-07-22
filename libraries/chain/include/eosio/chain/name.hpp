#pragma once
#include <string>
#include <fc/reflect/reflect.hpp>
#include <iosfwd>
#include <fc/uint256.hpp>

namespace eosio::chain {
  //using uint256_t = fc::uint256_t;
   struct name;
}
namespace fc {
  class variant;
  void to_variant(const eosio::chain::name& c, fc::variant& v);
  void from_variant(const fc::variant& v, eosio::chain::name& check);
} // fc

namespace eosio::chain {
   static constexpr fc::uint256_t char_to_symbol( char c ) {
      //mapstr ".-0123456789abcdefghijklmnopqrstuvwxyz_:<>[]{}()`~.............."
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

   static constexpr fc::uint256_t string_to_uint256_t( std::string_view str ) {
      fc::uint256_t name(0);
      int i = 0;
      for ( ; str[i] && i < 42; ++i) {
         // NOTE: char_to_symbol() returns char type, and without this explicit
         // expansion to uint64 type, the compilation fails at the point of usage
         // of string_to_name(), where the usage requires constant (compile time) expression.
         //name |= (char_to_symbol(str[i]) & 0x3f) << (64 - 6 * (i + 1));
         fc::uint256_t c = (char_to_symbol(str[i]) & 0x3f);
         c <<= (4 + (6 * i));
         name |= c;
      }

      // The for-loop encoded up to 60 high bits into uint64 'name' variable,
      // if (strlen(str) > 12) then encode str[12] into the low (remaining)
      // 4 bits of 'name'
      //if (i == 10)
      //    name |= char_to_symbol(str[10]) & 0x0F;
      return name;
      /*
      fc::uint256_t n = 0;
      int i = 0;
      for ( ; str[i] && i < 42; ++i) {
         // NOTE: char_to_symbol() returns char type, and without this explicit
         // expansion to uint256 type, the compilation fails at the point of usage
         // of string_to_name(), where the usage requires constant (compile time) expression.
         n |= (char_to_symbol(str[i]) & 0x3f) << (256 - 6 * (i + 1));
      }

      // The for-loop encoded up to 252 high bits into uint256 'name' variable,
      // if (strlen(str) > 42) then encode str[42] into the low (remaining)
      // 4 bits of 'name'
      if (i == 42)
         n |= char_to_symbol(str[42]) & 0x0F;
      return n;
      */
   }

   /// Immutable except for fc::from_variant.
   struct name {
   private:
      fc::uint256_t value = 0;

      friend struct fc::reflector<name>;
      friend void fc::from_variant(const fc::variant& v, eosio::chain::name& check);

      void set( std::string_view str );

   public:
      constexpr bool empty()const { return fc::uint256_t(0) == value; }
      constexpr bool good()const  { return !empty();   }

      explicit name( std::string_view str ) { set( str ); }
      constexpr explicit name( fc::uint256_t v ) : value(v) {}
      constexpr name() = default;

      name(const std::string& str) { set(str.c_str()); }
      name( const char* str )   { set(str); }

      std::string to_string()const;
      constexpr fc::uint256_t to_uint256_t()const { return value; }

      friend std::ostream& operator << ( std::ostream& out, const name& n ) {
         return out << n.to_string();
      }

      friend  bool operator < ( const name& a, const name& b ) { return a.value < b.value; }
      friend  bool operator > ( const name& a, const name& b ) { return a.value > b.value; }
      friend  bool operator <= ( const name& a, const name& b ) { return a.value <= b.value; }
      friend  bool operator >= ( const name& a, const name& b ) { return a.value >= b.value; }
      friend  bool operator == ( const name& a, const name& b ) { return a.value == b.value; }
      friend  bool operator != ( const name& a, const name& b ) { return a.value != b.value; }

      friend  bool operator == ( const name& a, fc::uint256_t b ) { return a.value == b; }
      friend  bool operator != ( const name& a, uint64_t b ) { return a.value != b; }

      friend bool operator == ( const fc::uint256_t& a, const name& b ) { return a == b.value; }
      friend bool operator != ( const fc::uint256_t& a, const name& b ) { return a != b.value; }

      constexpr explicit operator bool()const { return value != fc::uint256_t(0); }
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
   template<> struct hash<eosio::chain::name> : private hash<fc::uint256_t> {
      typedef eosio::chain::name argument_type;
      size_t operator()(const argument_type& name) const noexcept
      {
         return hash<fc::uint256_t>::operator()(name.to_uint256_t());
      }
   };
};

FC_REFLECT( eosio::chain::name, (value) )
