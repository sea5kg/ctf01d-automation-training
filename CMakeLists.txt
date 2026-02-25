cmake_minimum_required(VERSION 3.18)

project(ctf01d-automation-training)

include(${CMAKE_CURRENT_SOURCE_DIR}/src.wsjcpp/CMakeLists.txt)

#### BEGIN_WSJCPP_APPEND
#### END_WSJCPP_APPEND

set(CMAKE_CXX_STANDARD 17)
set(EXECUTABLE_OUTPUT_PATH ${ctf01d-automation-training_SOURCE_DIR})
# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# Sources

# include header dirs
list (APPEND WSJCPP_INCLUDE_DIRS "src")
list (APPEND WSJCPP_INCLUDE_DIRS "src/employees")
list (APPEND WSJCPP_INCLUDE_DIRS "src/database")
list (APPEND WSJCPP_INCLUDE_DIRS "src/handlers")

# handlers
list (APPEND WSJCPP_SOURCES "./src/handlers/request_handler.cpp")

# database
list (APPEND WSJCPP_SOURCES "./src/database/database_file.cpp")
list (APPEND WSJCPP_SOURCES "./src/database/db_uuids.cpp")
list (APPEND WSJCPP_SOURCES "./src/database/db_users.cpp")
list (APPEND WSJCPP_SOURCES "./src/database/db_rating.cpp")


# employees
list (APPEND WSJCPP_SOURCES "./src/employees/employ_config.cpp")
list (APPEND WSJCPP_SOURCES "./src/employees/employ_database.cpp")
list (APPEND WSJCPP_SOURCES "./src/employees/employ_uuids.cpp")
list (APPEND WSJCPP_SOURCES "./src/employees/employ_users.cpp")

# http_server
list (APPEND WSJCPP_SOURCES "src/web_server.cpp")

# main
list (APPEND WSJCPP_SOURCES "src/main.cpp")

# ZLIB
# find_package( ZLIB REQUIRED )
# if ( ZLIB_FOUND )
#     list (APPEND WSJCPP_INCLUDE_DIRS ${ZLIB_INCLUDE_DIRS})
#     include_directories( ${ZLIB_INCLUDE_DIRS} )
#     list (APPEND WSJCPP_LIBRARIES ${ZLIB_LIBRARIES})
# endif( ZLIB_FOUND )

# libhv
set(WITH_OPENSSL OFF CACHE BOOL "")
set(WITH_CURL OFF CACHE BOOL "")
# set(WITH_MQTT OFF CACHE BOOL "")
# set(WITH_EVPP OFF CACHE BOOL "")
set(BUILD_EXAMPLES OFF CACHE BOOL "")
# set(WITH_HTTP_CLIENT OFF CACHE BOOL "")
set(BUILD_SHARED OFF CACHE BOOL "")

add_subdirectory(src/3rdparty/libhv)
list (APPEND WSJCPP_LIBRARIES hv_static)
list (APPEND WSJCPP_INCLUDE_DIRS "${PROJECT_BINARY_DIR}/src/3rdparty/libhv/include/hv")

add_subdirectory(src/3rdparty/sqlite3)
# https://www.sqlite.org/threadsafe.html
# target_compile_definitions(sqlite3-static
#   PRIVATE
#     SQLITE_THREADSAFE=0
# )
list (APPEND WSJCPP_LIBRARIES sqlite3-static)
message(STATUS "sqlite3 includes dir: ${PROJECT_SOURCE_DIR}/src/3rdparty/sqlite3/src")
list (APPEND WSJCPP_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/src/3rdparty/sqlite3/src")

include_directories(${WSJCPP_INCLUDE_DIRS})

add_executable (ctf01d-automation-training ${WSJCPP_SOURCES})

target_link_libraries(ctf01d-automation-training -lpthread ${WSJCPP_LIBRARIES} )

install(
    TARGETS
        ctf01d-automation-training
    RUNTIME DESTINATION
        ${CMAKE_INSTALL_PREFIX}/bin
)

# enable testing functionality
# enable_testing()
# add_subdirectory(src/tests)
