#pragma once

namespace potato
{
    namespace chain
    {
        namespace config
        {
            static constexpr const uint32_t default_controller_thread_pool_size = 2;

            static constexpr const auto default_state_dir_name = "state";
            static constexpr const uint64_t default_state_size = 8*1024*1024*1024ull;
            static constexpr const uint64_t default_state_guard_size = 512*1024*1024ull;
        };
        
    }
}