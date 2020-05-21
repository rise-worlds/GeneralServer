add_executable( test_fc test_fc.cpp )
target_link_libraries( test_fc fc ${Boost_LIBRARIES})

add_test(NAME test_fc COMMAND tests/fc/test_fc WORKING_DIRECTORY ${CMAKE_BINARY_DIR})