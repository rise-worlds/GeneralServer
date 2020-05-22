#include <potato/chain/controller.hpp>
#include <potato/chain/types.hpp>

namespace potato
{
    namespace chain
    {
        struct controller_impl
        {
            const chain_id_type chain_id;
            controller_impl(const chain_id_type& chain_id)
            : chain_id(chain_id)
            {

            }
        };

        controller::controller( const chain_id_type& chain_id )
            : my(new controller_impl(chain_id))
        {

        }

        controller::~controller()
        {}

        chain_id_type controller::get_chain_id() const
        {
            return my->chain_id;
        }
    }
}