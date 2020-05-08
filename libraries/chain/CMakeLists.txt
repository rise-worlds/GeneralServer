file(GLOB HEADERS "include/potato/chain/*.hpp" )

add_library( potato_chain
            thread_utils.cpp
            ${HEADERS}
        )

target_link_libraries( potato_chain fc )
target_include_directories( potato_chain
            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
        )
