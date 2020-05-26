configure_file(${CMAKE_CURRENT_SOURCE_DIR}/genesis_state_root_key.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/genesis_state_root_key.cpp)

file(GLOB HEADERS "include/potato/chain/*.hpp" )

if(APPLE AND UNIX)
   set(PLATFORM_TIMER_IMPL platform_timer_macos.cpp)
else()
   try_run(POSIX_TIMER_TEST_RUN_RESULT POSIX_TIMER_TEST_COMPILE_RESULT ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/platform_timer_posix_test.c)
   if(POSIX_TIMER_TEST_RUN_RESULT EQUAL 0)
      set(PLATFORM_TIMER_IMPL platform_timer_posix.cpp)
      set(CHAIN_RT_LINKAGE rt)
   else()
      set(PLATFORM_TIMER_IMPL platform_timer_asio_fallback.cpp)
   endif()
endif()

add_library( potato_chain
            chain_config.cpp
            chain_id_type.cpp
            block_header.cpp
            controller.cpp
            genesis_state.cpp
            ${CMAKE_CURRENT_BINARY_DIR}/genesis_state_root_key.cpp
            name.cpp
            thread_utils.cpp
            transaction.cpp
            platform_timer_accuracy.cpp
            ${PLATFORM_TIMER_IMPL}
            ${HEADERS}
        )

target_link_libraries( potato_chain fc chainbase )
target_include_directories( potato_chain
            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
        )
