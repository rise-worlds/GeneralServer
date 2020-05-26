#pragma once

#include <potato/chain/chain_id_type.hpp>
#include <potato/chain/types.hpp>
#include <potato/chain/config.hpp>
#include <potato/chain/fork_database.hpp>
#include <potato/chain/genesis_state.hpp>

#include <chainbase/pinnable_mapped_file.hpp>
#include <chainbase/chainbase.hpp>
#include <boost/signals2/signal.hpp>
#include <fc/filesystem.hpp>

namespace potato::chain
{
    struct controller_impl;

    class controller
    {
    public:
        struct config
        {
            uint16_t                 thread_pool_size       =  chain::config::default_controller_thread_pool_size;
            path                     blocks_dir             =  chain::config::default_blocks_dir_name;
            path                     state_dir              =  chain::config::default_state_dir_name;
            uint64_t                 state_size             =  chain::config::default_state_size;
            uint64_t                 state_guard_size       =  chain::config::default_state_guard_size;
            uint64_t                 reversible_cache_size  =  chain::config::default_reversible_cache_size;
            uint64_t                 reversible_guard_size  =  chain::config::default_reversible_guard_size;
        };
    
        controller(const config& config, const chain_id_type &chain_id);
        ~controller();

        void startup(std::function<bool()> shutdown, const genesis_state& genesis);
        void startup(std::function<bool()> shutdown);

        const chainbase::database& db()const;
        const fork_database& fork_db()const;

        chain_id_type get_chain_id() const;
        boost::asio::io_context& get_thread_pool();

        static fc::optional<chain_id_type> extract_chain_id_from_db( const path& state_dir );

    private:
        std::unique_ptr<controller_impl> my;

        chainbase::database& mutable_db()const;
    };
} // namespace potato::chain