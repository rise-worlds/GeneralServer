#include <appbase/application.hpp>

#include <eosio/http_plugin/http_plugin.hpp>
#include <eosio/net_plugin/net_plugin.hpp>
#include <eosio/producer_plugin/producer_plugin.hpp>
#include <potato/version/version.hpp>

#include <boost/exception/diagnostic_information.hpp>
#include <fc/filesystem.hpp>
#include <fc/log/logger_config.hpp>
#include <fc/log/appender.hpp>
#include <fc/exception/exception.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/compressed_pair.hpp>
#include <boost/dll/runtime_symbol_info.hpp>
#include <boost/exception/diagnostic_information.hpp>

#include "config.hpp"

using namespace appbase;
using namespace eosio;

void logging_conf_handler()
{
   auto config_path = app().get_logging_conf();
   try
   {
      if (fc::exists(config_path))
      {
         ilog("Received HUP.  Reloading logging configuration from ${p}.", ("p", config_path.string()));
         fc::configure_logging(config_path);
      }
      else
      {
         ilog("Received HUP.  No log config found at ${p}, setting to default.", ("p", config_path.string()));
         fc::configure_logging(fc::logging_config::default_config());
      }
   }
   catch (...)
   {
      elog("Error reloading logging.json");
      throw;
   }
   fc::log_config::initialize_appenders(app().get_io_service());
}

void initialize_logging()
{
   auto config_path = app().get_logging_conf();
   if (fc::exists(config_path))
      fc::configure_logging(config_path); // intentionally allowing exceptions to escape
   fc::log_config::initialize_appenders(app().get_io_service());

   app().set_sighup_callback(logging_conf_handler);
}

int main(int argc, char **argv)
{
   try
   {
      app().set_version(GeneralServer::nodeos::config::version);
      app().set_version_string(potato::version::version_client());
      app().set_full_version_string(potato::version::version_full());

      // fc::logger::get(DEFAULT_LOGGER).set_log_level(fc::log_level::debug);

      http_plugin::set_defaults({.default_unix_socket_path = "",
                                 .default_http_port = 8888});
      if (!app().initialize<chain_plugin, net_plugin, producer_plugin>(argc, argv))
      {
         const auto &opts = app().get_options();
         if (opts.count("help") || opts.count("version") || opts.count("full-version") || opts.count("print-default-config"))
         {
            return 0;
         }
         return -1;
      }
      initialize_logging();
      ilog("nodeos version ${ver} ${fv}",
           ("ver", app().version_string())("fv", app().version_string() == app().full_version_string() ? "" : app().full_version_string()));
      ilog("nodeos using configuration file ${c}", ("c", app().full_config_file_path().string()));
      ilog("nodeos data directory is ${d}", ("d", app().data_dir().string()));
      ilog("nodeos log file is ${d}", ("d", app().get_logging_conf().string()));
      app().startup();
      app().set_thread_priority_max();
      app().exec();
   }
   catch (const extract_genesis_state_exception &e)
   {
      return 3;
   }
   catch (const fixed_reversible_db_exception &e)
   {
      return 4;
   }
   catch (const node_management_success &e)
   {
      return 5;
   }
   catch (const fc::exception &e)
   {
      if (e.code() == fc::std_exception_code)
      {
         if (e.top_message().find("database dirty flag set") != std::string::npos)
         {
            elog("database dirty flag set (likely due to unclean shutdown): replay required");
            return 2;
         }
      }
      elog("${e}", ("e", e.to_detail_string()));
      return -2;
   }
   catch (const boost::interprocess::bad_alloc &e)
   {
      elog("bad alloc");
      return 1;
   }
   catch (const boost::exception &e)
   {
      elog("${e}", ("e", boost::diagnostic_information(e)));
      return -2;
   }
   catch (const std::runtime_error &e)
   {
      if (std::string(e.what()).find("database dirty flag set") != std::string::npos)
      {
         elog("database dirty flag set (likely due to unclean shutdown): replay required");
         return 2;
      }
      else
      {
         elog("${e}", ("e", e.what()));
      }
      return -2;
   }
   catch (const std::exception &e)
   {
      elog("${e}", ("e", e.what()));
      return -2;
   }
   catch (...)
   {
      elog("unknown exception");
      return -2;
   }

   ilog("nodeos successfully exiting");
   return 0;
}