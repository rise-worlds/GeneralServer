#pragma once
#include <potato/chain/types.hpp>
#include <potato/chain/block_timestamp.hpp>
#include "multi_index_includes.hpp"

namespace potato::chain
{
    class account_object : public chainbase::object<account_object_type, account_object>
    {
        OBJECT_CTOR(account_object, (abi))

        id_type id;
        account_name name; //< name should not be changed within a chainbase modifier lambda
        block_timestamp_type creation_date;
        shared_blob abi;

        void set_abi(const eosio::chain::abi_def &a)
        {
            abi.resize(fc::raw::pack_size(a));
            fc::datastream<char *> ds(abi.data(), abi.size());
            fc::raw::pack(ds, a);
        }

        eosio::chain::abi_def get_abi() const
        {
            eosio::chain::abi_def a;
            EOS_ASSERT(abi.size() != 0, abi_not_found_exception, "No ABI set on account ${n}", ("n", name));

            fc::datastream<const char *> ds(abi.data(), abi.size());
            fc::raw::unpack(ds, a);
            return a;
        }
    };
    using account_id_type = account_object::id_type;

    struct by_name;
    using account_index = chainbase::shared_multi_index_container<
        account_object,
        indexed_by<
            ordered_unique<tag<by_id>, member<account_object, account_object::id_type, &account_object::id>>,
            ordered_unique<tag<by_name>, member<account_object, account_name, &account_object::name>>
        >
    >;
} // namespace potato::chain
CHAINBASE_SET_INDEX_TYPE(potato::chain::account_object, potato::chain::account_index)

FC_REFLECT(potato::chain::account_object, (name)(creation_date)(abi))
