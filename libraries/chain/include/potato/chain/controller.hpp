#pragma once

#include <potato/chain/chain_id_type.hpp>
#include <potato/chain/types.hpp>
#include <potato/chain/config.hpp>

#include <chainbase/pinnable_mapped_file.hpp>
#include <boost/signals2/signal.hpp>
#include <fc/filesystem.hpp>

namespace potato
{
    namespace chain
    {
        struct controller_impl;

        class controller
        {
        public:
            struct config
            {
                path                     state_dir              =  chain::config::default_state_dir_name;
                uint64_t                 state_size             =  chain::config::default_state_size;
                uint64_t                 state_guard_size       =  chain::config::default_state_guard_size;
            };
        
            controller(const config& config, const chain_id_type &chain_id);
            ~controller();

            chain_id_type get_chain_id() const;

        private:
            std::unique_ptr<controller_impl> my;
        };
    } // namespace chain
} // namespace potato