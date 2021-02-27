cmake_minimum_required(VERSION 3.1)
project(luawav)

option(BUILD_SHARED_LIBS "Build modules as shared libraries" ON)

find_package(PkgConfig)
include(FindPackageHandleStandardArgs)

if(LUA_VERSION)
  find_package(Lua ${LUA_VERSION} EXACT REQUIRED)
else()
  find_package(Lua REQUIRED)
  set(LUA_VERSION "${LUA_VERSION_MAJOR}.${LUA_VERSION_MINOR}" CACHE STRING "Lua version to use")
endif()

set(CMODULE_INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib/lua/${LUA_VERSION}")
set(LUAMODULE_INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/share/lua/${LUA_VERSION}")

configure_file(
  "src/luawav/version.lua"
  "${CMAKE_BINARY_DIR}/luawav/version.lua"
  COPYONLY
)

install(FILES "src/luawav/version.lua"
  DESTINATION "${LUAMODULE_INSTALL_LIB_DIR}/luawav/"
)

set(luawav_sources)
list(APPEND luawav_sources "csrc/luawav.c")
list(APPEND luawav_sources "csrc/luawav_int64.c")
list(APPEND luawav_sources "csrc/luawav_internal.c")
list(APPEND luawav_sources "csrc/dr_wav.c")

add_library(luawav ${luawav_sources})

target_link_libraries(luawav PRIVATE ${OPUS_LIBRARIES})
target_link_directories(luawav PRIVATE ${OPUS_LIBRARY_DIRS})
if(WIN32)
    target_link_libraries(luawav PRIVATE ${LUA_LIBRARIES})
endif()
target_include_directories(luawav PRIVATE ${OPUS_INCLUDEDIR})
target_include_directories(luawav PRIVATE ${LUA_INCLUDE_DIR})

if(APPLE)
    set(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "${CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS} -undefined dynamic_lookup")
    if(BUILD_SHARED_LIBS)
        set_target_properties(luawav PROPERTIES SUFFIX ".so")
    endif()
endif()

set_target_properties(luawav PROPERTIES PREFIX "")
set_target_properties(luawav PROPERTIES OUTPUT_NAME "luawav")
set_target_properties(luawav PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set_target_properties(luawav PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set_target_properties(luawav PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
  string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
  set_target_properties(luawav PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_BINARY_DIR}")
  set_target_properties(luawav PROPERTIES LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_BINARY_DIR}")
  set_target_properties(luawav PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_BINARY_DIR}")
endforeach()

install(TARGETS luawav
  LIBRARY DESTINATION "${CMODULE_INSTALL_LIB_DIR}"
  RUNTIME DESTINATION "${CMODULE_INSTALL_LIB_DIR}"
  ARCHIVE DESTINATION "${CMODULE_INSTALL_LIB_DIR}"
)
