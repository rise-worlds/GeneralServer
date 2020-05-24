#include <potato/chain/controller.hpp>
#include <potato/chain/types.hpp>

#include <chainbase/chainbase.hpp>

namespace potato
{
    using chainbase::database;

    namespace chain
    {
        struct controller_impl
        {
            const chain_id_type chain_id;

            controller::config config;
            chainbase::database db;

            controller_impl(const controller::config &config, const chain_id_type &chain_id)
                : chain_id(chain_id)
                , config(config)
                , db(config.state_dir)
            {
            }
        };

        controller::controller(const controller::config &config, const chain_id_type &chain_id)
            : my(new controller_impl(config, chain_id))
        {
        }

        controller::~controller()
        {
        }

        chain_id_type controller::get_chain_id() const
        {
            return my->chain_id;
        }
    } // namespace chain
} // namespace potato