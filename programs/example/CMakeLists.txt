add_executable( example main.cpp )

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

target_link_libraries( example appbase fc wabt ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS} )
