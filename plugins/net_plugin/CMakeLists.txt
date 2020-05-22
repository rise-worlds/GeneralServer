file(GLOB HEADERS "include/potato/net_plugin/*.hpp")
add_library( net_plugin
             net_plugin.cpp
             ${HEADERS} )

target_link_libraries( net_plugin appbase fc potato_chain chain_plugin producer_plugin )
target_include_directories( net_plugin PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
