#include <boost/hana.hpp>

#include "name.hpp"
#include "namev2.hpp"

namespace hana = boost::hana;

namespace example
{
    template<uint128_t TableName_L, uint128_t TableName_R, typename Extractor>
    struct indexed_by {
        enum constants { index_name   = static_cast<name>(name(TableName_L, TableName_R)) };
        typedef Extractor secondary_extractor_type;
    };

//    template<example::name& TableName, typename TYPE, typename... Indices>
//    class muilt_index {
//    private:
////        using TableName = std::string_view{example::detail::to_const_char_arr<Str...>::value, sizeof...(Str)};
//        static_assert( sizeof...(Indices) <= 16, "multi_index only supports a maximum of 16 secondary indices" );
//
////        std::string_view TableName = std::string_view{example::detail::to_const_char_arr<Str...>::value, sizeof...(Str)};
//        constexpr static bool validate_table_name( name n ) {
//            return n.length() < 43; //(n & 0x000000000000000FULL) == 0;
//        }
//        static_assert( validate_table_name( name(TableName) ), "multi_index does not support table names with a length greater than 43");
//
//        name     _code;
//        name     _scope;
//    public:
//        muilt_index( name code, name scope ) : _code(code), _scope(scope) {}
//    };

    template<uint128_t TableName_L, uint128_t TableName_R, typename TYPE, typename... Indices>
    class muilt_index {
    private:
//        using TableName = std::string_view{example::detail::to_const_char_arr<Str...>::value, sizeof...(Str)};
        static_assert( sizeof...(Indices) <= 16, "multi_index only supports a maximum of 16 secondary indices" );

//        std::string_view TableName = std::string_view{example::detail::to_const_char_arr<Str...>::value, sizeof...(Str)};
        constexpr static bool validate_table_name( name n ) {
            return n.length() < 43; //(n & 0x000000000000000FULL) == 0;
        }
        static_assert( validate_table_name( name(TableName_L, TableName_R) ), "multi_index does not support table names with a length greater than 43");
    public:
        static constexpr name TableName = name(TableName_L, TableName_R);
        name     _code;
        name     _table = name(TableName_L, TableName_R);
        name     _scope;
    public:
        muilt_index( name code, name scope ) : _code(code), _scope(scope) {}

        template<uint128_t IndexName_L, uint128_t IndexName_R, typename Extractor, uint64_t Number, bool IsConst>
        struct index {
            static constexpr name IndexName = name(IndexName_L, IndexName_R);
        public:
            typedef Extractor  secondary_extractor_type;
            typedef typename std::decay<decltype( Extractor()(nullptr) )>::type secondary_key_type;

            constexpr static bool validate_index_name( name n ) {
                return n != example::name(0, 0) && n != example::name("primary"); // Primary is a reserve index name.
            }

            static_assert( validate_index_name( name(IndexName_L, IndexName_R) ), "invalid index name used in multi_index" );

            enum constants {
                table_name   = static_cast<name>(TableName),
                index_name   = static_cast<name>(name(IndexName_L, IndexName_R)),
                index_number = Number,
                index_table_name = static_cast<name>(TableName)
//               index_table_name = (static_cast<uint64_t>(TableName) & 0xFFFFFFFFFFFFFFF0ULL)
//                                    | (Number & 0x000000000000000FULL) // Assuming no more than 16 secondary indices are allowed
            };

            constexpr static uint64_t name()   { return index_table_name; }
            constexpr static uint64_t number() { return Number; }
        };
    };

    template<const char* TableNameStr, typename TYPE, typename... Indices>
    class muilt_index_test {
    private:
//        using TableName = std::string_view{example::detail::to_const_char_arr<Str...>::value, sizeof...(Str)};
        static_assert( sizeof...(Indices) <= 16, "multi_index only supports a maximum of 16 secondary indices" );

//        std::string_view TableName = std::string_view{example::detail::to_const_char_arr<Str...>::value, sizeof...(Str)};
        constexpr static bool validate_table_name( name n ) {
            return n.length() < 43; //(n & 0x000000000000000FULL) == 0;
        }
        static_assert( validate_table_name( name(TableNameStr) ), "multi_index does not support table names with a length greater than 43");
    public:
        static constexpr name TableName = name(TableNameStr);
        name     _code;
        name     _table = name(TableName);
        name     _scope;
    public:
        muilt_index_test( name code, name scope ) : _code(code), _scope(scope) {}

        template<uint128_t IndexName_L, uint128_t IndexName_R, typename Extractor, uint64_t Number, bool IsConst>
        struct index {
            static constexpr name IndexName = name(IndexName_L, IndexName_R);
        public:
            typedef Extractor secondary_extractor_type;
            typedef typename std::decay<decltype( Extractor()(nullptr) )>::type secondary_key_type;

            constexpr static bool validate_index_name( name n ) {
                return n != example::name(0, 0) && n != example::name("primary"); // Primary is a reserve index name.
            }

            static_assert( validate_index_name( name(IndexName_L, IndexName_R) ), "invalid index name used in multi_index" );

            enum constants {
                table_name   = static_cast<name>(TableName),
                index_name   = static_cast<name>(name(IndexName_L, IndexName_R)),
                index_number = Number,
                index_table_name = static_cast<name>(TableName)
//               index_table_name = (static_cast<uint64_t>(TableName) & 0xFFFFFFFFFFFFFFF0ULL)
//                                    | (Number & 0x000000000000000FULL) // Assuming no more than 16 secondary indices are allowed
            };

            constexpr static uint64_t name()   { return index_table_name; }
            constexpr static uint64_t number() { return Number; }
        };
    };
}