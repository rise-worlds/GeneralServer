#pragma once
#include <fc/array.hpp>

#include <potato/chain/types.hpp>
#include <potato/chain/chain_config.hpp>
#include <chainbase/chainbase.hpp>

#include "multi_index_includes.hpp"

namespace potato
{
    namespace chain
    {
        class global_property_object : public chainbase::object<global_property_object_type, global_property_object>
        {
            OBJECT_CTOR(global_property_object)
            public:
            id_type         id;
            chain_config    configuration;
            chain_id_type   chain_id;
        };

        using global_property_multi_index = chainbase::shared_multi_index_container<
            global_property_object,
            indexed_by<
                ordered_unique<tag<by_id>,
                    BOOST_MULTI_INDEX_MEMBER(global_property_object, global_property_object::id_type, id)
                >
            >
        >;

        /**
        * @class dynamic_global_property_object
        * @brief Maintains global state information that frequently change
        * @ingroup object
        * @ingroup implementation
        */
        class dynamic_global_property_object : public chainbase::object<dynamic_global_property_object_type, dynamic_global_property_object>
        {
            OBJECT_CTOR(dynamic_global_property_object)

            id_type    id;
            uint64_t   global_action_sequence = 0;
        };

        using dynamic_global_property_multi_index = chainbase::shared_multi_index_container<
            dynamic_global_property_object,
            indexed_by<
                ordered_unique<tag<by_id>,
                    BOOST_MULTI_INDEX_MEMBER(dynamic_global_property_object, dynamic_global_property_object::id_type, id)
                >
            >
        >;
    }
}

CHAINBASE_SET_INDEX_TYPE(potato::chain::global_property_object, potato::chain::global_property_multi_index)
CHAINBASE_SET_INDEX_TYPE(potato::chain::dynamic_global_property_object,
                         potato::chain::dynamic_global_property_multi_index)

FC_REFLECT(potato::chain::global_property_object,
            // (proposed_schedule_block_num)(proposed_schedule)
            (configuration)(chain_id)
          )
FC_REFLECT(potato::chain::dynamic_global_property_object,
            (global_action_sequence)
          )