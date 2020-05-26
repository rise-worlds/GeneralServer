#pragma once

#include <fc/crypto/sha256.hpp>

namespace potato
{
    class chain_plugin;
    class net_plugin_impl;
    struct handshake_message;

    namespace chain
    {
        struct chain_id_type : public fc::sha256
        {
            using fc::sha256::sha256;
            template <typename T>
            inline friend T &operator<<(T &ds, const chain_id_type &cid)
            {
                ds.write(cid.data(), cid.data_size());
                return ds;
            }

            template <typename T>
            inline friend T &operator>>(T &ds, chain_id_type &cid)
            {
                ds.read(cid.data(), cid.data_size());
                return ds;
            }

        private:
            chain_id_type() = default;

            template<typename T>
            friend T fc::variant::as()const;

            friend class potato::chain_plugin;
            friend class potato::net_plugin_impl;
            friend struct potato::handshake_message;
            friend class controller;
            friend struct controller_impl;
            friend class global_property_object;
        };
    } // namespace chain
} // namespace potato

namespace fc
{
    class variant;
    void to_variant(const potato::chain::chain_id_type &cid, fc::variant &v);
    void from_variant(const fc::variant &v, potato::chain::chain_id_type &cid);
} // namespace fc