#include <boost/hana.hpp>

namespace example
{
namespace hana = boost::hana;
template <class Class, typename Type, Type (Class::*PtrToMemberFunction)() const>
struct const_mem_fun
{
    typedef typename std::remove_reference<Type>::type result_type;

    template <typename ChainedPtr>

    auto operator()(const ChainedPtr &x) const -> std::enable_if_t<!std::is_convertible<const ChainedPtr &, const Class &>::value, Type>
    {
        return operator()(*x);
    }

    Type operator()(const Class &x) const
    {
        return (x.*PtrToMemberFunction)();
    }

    Type operator()(const std::reference_wrapper<const Class> &x) const
    {
        return operator()(x.get());
    }

    Type operator()(const std::reference_wrapper<Class> &x) const
    {
        return operator()(x.get());
    }
};
template <uint64_t IndexName, uint64_t IndexNameR, typename Extractor>
struct indexed_by
{
    constexpr static uint64_t index_name = IndexName;
    constexpr static uint64_t index_name_r = IndexNameR;
    // enum constants { index_name   = static_cast<uint64_t>(IndexName) };
    typedef Extractor secondary_extractor_type;
};
template <uint64_t TableName, typename T, typename... Indices>
class multi_index
{
public:
    static_assert(sizeof...(Indices) <= 16, "");
    // constexpr static uint64_t table_name = TableName;
    enum constants { table_name   = static_cast<uint64_t>(TableName) };
    uint64_t _code, _scope;

    multi_index(uint64_t code, uint64_t scope) : _code(code),_scope(scope) {}

    template <uint64_t IndexName, uint64_t IndexNameR, typename Extractor, uint64_t Number, bool IsConst>
    class index
    {
    public:
        typedef Extractor secondary_extractor_type;
        typedef typename std::decay<decltype(Extractor()(nullptr))>::type secondary_key_type;
        constexpr static uint64_t table_name = TableName;
        constexpr static uint64_t index_name = IndexName;
        constexpr static uint64_t number = Number;
        // enum constants {
        //     table_name   = static_cast<uint64_t>(TableName),
        //     index_name   = static_cast<uint64_t>(IndexName),
        //     number       = static_cast<uint64_t>(Number),
        // };
    };
    template <uint64_t I>
    struct intc { enum e{ value = I }; operator uint64_t() const { return I; }  };
    static constexpr auto transform_indices()
    {
        typedef decltype(hana::zip_shortest(
            hana::make_tuple(intc<0>(), intc<1>(), intc<2>(), intc<3>(), intc<4>(), intc<5>(),
                             intc<6>(), intc<7>(), intc<8>(), intc<9>(), intc<10>(), intc<11>(),
                             intc<12>(), intc<13>(), intc<14>(), intc<15>()),
        hana::tuple<Indices...>())) indices_input_type;
        return hana::transform( indices_input_type(), [&]( auto&& idx ){
            typedef typename std::decay<decltype(hana::at_c<0>(idx))>::type num_type;
            typedef typename std::decay<decltype(hana::at_c<1>(idx))>::type idx_type;
            return hana::make_tuple(hana::type_c<index<idx_type::index_name, idx_type::index_name_r,
                                                       typename idx_type::secondary_extractor_type,
                                                       num_type::e::value, false>>,
                                    hana::type_c<index<idx_type::index_name, idx_type::index_name_r,
                                                       typename idx_type::secondary_extractor_type,
                                                       num_type::e::value, true>>);
                    });
    }
    typedef decltype( multi_index::transform_indices() ) indices_type;

    indices_type _indices;
};
} // namespace example