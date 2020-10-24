cmake_minimum_required(VERSION 3.10)

set (CMAKE_CONFIGURATION_TYPES "Debug;Release")

project(lwIP)

# Example lwIP application
set(LWIP_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set (LWIP_DEFINITIONS LWIP_DEBUG=1)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  add_subdirectory(${LWIP_DIR}/contrib/ports/win32/example_app)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
  add_subdirectory(${LWIP_DIR}/contrib/ports/unix/example_app)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Android")
  include(${LWIP_DIR}/contrib/ports/CMakeCommon.cmake)
  set (LWIP_INCLUDE_DIRS
      "${LWIP_DIR}/src/include"
      "${LWIP_DIR}/contrib/"
      "${LWIP_DIR}/contrib/ports/unix/port/include"
      "${LWIP_DIR}/contrib/examples/example_app"
  )
  include(${LWIP_DIR}/src/Filelists.cmake)
  include(${LWIP_DIR}/contrib/Filelists.cmake)
  include(${LWIP_DIR}/contrib/ports/unix/Filelists.cmake)
else()
  message(WARNING "Host ${CMAKE_SYSTEM_NAME} is not supported to build example_app")
endif()

# Source package generation
set(CPACK_SOURCE_GENERATOR "ZIP")
set(CPACK_SOURCE_PACKAGE_DESCRIPTION_SUMMARY "lwIP lightweight IP stack")
set(CPACK_PACKAGE_VERSION_MAJOR "${LWIP_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${LWIP_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${LWIP_VERSION_REVISION}")
set(CPACK_SOURCE_IGNORE_FILES "/build/;${CPACK_SOURCE_IGNORE_FILES};.git")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "lwip-${LWIP_VERSION_MAJOR}.${LWIP_VERSION_MINOR}.${LWIP_VERSION_REVISION}")
include(CPack)

# Generate docs before creating source package
include(src/Filelists.cmake)
add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)
if (TARGET lwipdocs)
  add_dependencies(dist lwipdocs)
endif()
