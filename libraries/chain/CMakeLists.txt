configure_file(${CMAKE_CURRENT_SOURCE_DIR}/genesis_state_root_key.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/genesis_state_root_key.cpp)

file(GLOB HEADERS "include/potato/chain/*.hpp" )

add_library( potato_chain
            chain_id_type.cpp
            controller.cpp
            genesis_state.cpp
            ${CMAKE_CURRENT_BINARY_DIR}/genesis_state_root_key.cpp
            thread_utils.cpp
            ${HEADERS}
        )

target_link_libraries( potato_chain fc chainbase )
target_include_directories( potato_chain
            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
        )
