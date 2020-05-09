#pragma once
#include <potato/chain_plugin/chain_plugin.hpp>

#include <appbase/application.hpp>

#include <boost/container/flat_set.hpp>
#include <boost/multiprecision/cpp_int.hpp>

#include <fc/static_variant.hpp>
namespace fc { class variant; }

namespace potato
{
    using namespace appbase;
    class producer_plugin : public appbase::plugin<producer_plugin>
    {
    public:
        producer_plugin();
        virtual ~producer_plugin();

        APPBASE_PLUGIN_REQUIRES((chain_plugin))
        virtual void set_program_options(options_description &cli, options_description &cfg) override;
        void handle_sighup() override;

        void plugin_initialize(const variables_map &options);
        void plugin_startup();
        void plugin_shutdown();

    private:
        std::shared_ptr<class producer_plugin_impl> my;
    };

} // namespace potato