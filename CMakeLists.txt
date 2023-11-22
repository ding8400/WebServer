# This file handles building each task as sub-project.
#
# Each task becomes an independent project in which you can experiment.
#
# Tasks are added as separate sub-projects so as compilation errors in
# one task don't affect another task.

cmake_minimum_required(VERSION 3.22)
project(webserver)

set(CMAKE_CXX_STANDARD 17)

add_executable(http_server main.cpp
        config.cpp
        webserver.cpp
        ./timer/lst_timer.cpp
        ./http/http_conn.cpp
        ./log/log.cpp
        ./CGImysql/sql_connection_pool.cpp
)

target_link_libraries(http_server mysqlclient pthread)