#pragma once

#include <potato/chain/chain_id_type.hpp>

#include <chainbase/pinnable_mapped_file.hpp>
#include <boost/signals2/signal.hpp>

namespace potato
{
    namespace chain
    {
        struct controller_impl;

        class controller
        {
            public:
            controller( const chain_id_type& chain_id );
            ~controller();
            
            chain_id_type get_chain_id()const;

            private:
            std::unique_ptr<controller_impl> my;
        };
    }
}