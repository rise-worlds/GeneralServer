add_executable( example main.cpp )

target_include_directories(example PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries( example appbase fc ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS} )
