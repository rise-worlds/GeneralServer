add_executable( example main.cpp )

if( UNIX AND NOT APPLE )
  set(rt_library rt )
endif()

target_include_directories(example PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

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
    PRIVATE appbase 
    PRIVATE -Wl,${whole_archive_flag} net_plugin -Wl,${no_whole_archive_flag}
    PRIVATE -Wl,${build_id_flag}
    PRIVATE http_plugin chain_plugin producer_plugin
    PRIVATE potato_chain fc ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS} )
