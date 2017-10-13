#
# CMakeLists.txt  cmake file for mdcs library
# 10-Oct-2017  chuck@ece.cmu.edu
#

# list of source files
set(mdcs-src mdcs-service.c mdcs-client.c mdcs-counters.c mdcs-rpc.c
    mdcs-hash-string.c)

# load package helper for generating cmake CONFIG packages
include (CMakePackageConfigHelpers)

# where to install files for "find_package"
set (mdcs-pkg "share/cmake/mdcs")

#
# library version set here (e.g. for shared libs).
#
set (MDCS_VERSION_MAJOR 1)
set (MDCS_VERSION_MINOR 0)
set (MDCS_VERSION_PATCH 0)
set (mdcs-vers "${MDCS_VERSION_MAJOR}.${MDCS_VERSION_MINOR}")
set (MDCS_VERSION "${mdcs-vers}.${MDCS_VERSION_PATCH}")

add_library(mdcs ${mdcs-src})
target_link_libraries (mdcs mercury margo)
target_include_directories (mdcs PUBLIC $<INSTALL_INTERFACE:include>)

# local include's BEFORE, in case old incompatable .h files in prefix/include
target_include_directories (mdcs BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)

# for shared libs, establish the lib version
set_target_properties (mdcs
    PROPERTIES VERSION ${MDCS_VERSION}
    SOVERSION ${MDCS_VERSION_MAJOR})

#
# installation stuff (packaging and install commands)
#
write_basic_package_version_file(
    "mdcs-config-version.cmake"
    VERSION ${MDCS_VERSION}
    COMPATIBILITY AnyNewerVersion)

# generate our config file for find_package()
configure_file (mdcs-config.cmake.in mdcs-config.cmake @ONLY)

#
# "make install" rules
#
install (TARGETS mdcs EXPORT mdcs-targets
         ARCHIVE DESTINATION lib
         LIBRARY DESTINATION lib)
install (EXPORT mdcs-targets
         DESTINATION ${mdcs-pkg}
         FILE "mdcs-targets.cmake")
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/mdcs-config.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/mdcs-config-version.cmake"
               "../cmake/xpkg-import.cmake"
         DESTINATION ${mdcs-pkg} )
install (DIRECTORY ../include/mdcs
         DESTINATION include
         FILES_MATCHING PATTERN "*.h")
