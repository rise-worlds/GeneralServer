add_executable( example main.cpp )

if( UNIX AND NOT APPLE )
  set(rt_library rt )
endif()

target_include_directories(example PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
configure_file(config.hpp.in config.hpp ESCAPE_QUOTES)

target_include_directories(example 
                          PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
                          ${CMAKE_SOURCE_DIR}/libraries/wabt
                          ${CMAKE_BINARY_DIR}/libraries/wabt
                          )

if(UNIX)
  if(APPLE)
    set(whole_archive_flag "-force_load")
    set(no_whole_archive_flag "")
    set(build_id_flag "")
  else()
    set(whole_archive_flag "--whole-archive")
    set(no_whole_archive_flag "--no-whole-archive")
    set(build_id_flag "--build-id")
  endif()
else()
  set(whole_archive_flag "--whole-archive")
  set(no_whole_archive_flag "--no-whole-archive")
  set(build_id_flag "")
endif()

target_link_libraries( example 
    PRIVATE appbase version
    PRIVATE -Wl,${whole_archive_flag} net_plugin          -Wl,${no_whole_archive_flag}
    PRIVATE -Wl,${whole_archive_flag} chain_api_plugin    -Wl,${no_whole_archive_flag}
    PRIVATE -Wl,${whole_archive_flag} producer_api_plugin -Wl,${no_whole_archive_flag}
    PRIVATE -Wl,${build_id_flag}
    PRIVATE http_plugin chain_plugin producer_plugin
    PRIVATE eosio_chain fc wabt ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS} )
