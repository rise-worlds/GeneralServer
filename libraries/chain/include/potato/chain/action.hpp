#pragma once
#include <potato/chain/types.hpp>
#include <potato/chain/exceptions.hpp>

namespace potato::chain
{
    struct permission_level
    {
        account_name actor;
        permission_name permission;
    };

    inline bool operator==(const permission_level &lhs, const permission_level &rhs)
    {
        return std::tie(lhs.actor, lhs.permission) == std::tie(rhs.actor, rhs.permission);
    }

    inline bool operator!=(const permission_level &lhs, const permission_level &rhs)
    {
        return std::tie(lhs.actor, lhs.permission) != std::tie(rhs.actor, rhs.permission);
    }

    inline bool operator<(const permission_level &lhs, const permission_level &rhs)
    {
        return std::tie(lhs.actor, lhs.permission) < std::tie(rhs.actor, rhs.permission);
    }

    inline bool operator<=(const permission_level &lhs, const permission_level &rhs)
    {
        return std::tie(lhs.actor, lhs.permission) <= std::tie(rhs.actor, rhs.permission);
    }

    inline bool operator>(const permission_level &lhs, const permission_level &rhs)
    {
        return std::tie(lhs.actor, lhs.permission) > std::tie(rhs.actor, rhs.permission);
    }

    inline bool operator>=(const permission_level &lhs, const permission_level &rhs)
    {
        return std::tie(lhs.actor, lhs.permission) >= std::tie(rhs.actor, rhs.permission);
    }

    struct action
    {
        account_name account;
        action_name name;
        vector<permission_level> authorization;
        bytes data;

        action() {}

        template <typename T, std::enable_if_t<std::is_base_of<bytes, T>::value, int> = 1>
        action(vector<permission_level> auth, const T &value)
        {
            account = T::get_account();
            name = T::get_name();
            authorization = move(auth);
            data.assign(value.data(), value.data() + value.size());
        }

        template <typename T, std::enable_if_t<!std::is_base_of<bytes, T>::value, int> = 1>
        action(vector<permission_level> auth, const T &value)
        {
            account = T::get_account();
            name = T::get_name();
            authorization = move(auth);
            data = fc::raw::pack(value);
        }

        action(vector<permission_level> auth, account_name account, action_name name, const bytes &data)
            : account(account), name(name), authorization(move(auth)), data(data)
        {
        }

        template <typename T>
        T data_as() const
        {
            EOS_ASSERT(account == T::get_account(), action_type_exception, "account is not consistent with action struct");
            EOS_ASSERT(name == T::get_name(), action_type_exception, "action name is not consistent with action struct");
            return fc::raw::unpack<T>(data);
        }
    };
} // namespace potato::chain

FC_REFLECT(potato::chain::permission_level, (actor)(permission))
FC_REFLECT(potato::chain::action, (account)(name)(authorization)(data))
