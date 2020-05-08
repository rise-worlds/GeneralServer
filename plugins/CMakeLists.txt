add_subdirectory(http_plugin)

# Forward variables to top level so packaging picks them up
set(CPACK_DEBIAN_PACKAGE_DEPENDS ${CPACK_DEBIAN_PACKAGE_DEPENDS} PARENT_SCOPE)
