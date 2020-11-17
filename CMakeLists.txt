cmake_minimum_required(VERSION 2.8.11)
project(a-gnucash-integration)
add_definitions(-DHAVE_SCANF_LLD) # to satisfy gnucash headers
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # for clang-tidy
include_directories(/usr/include/glib-2.0 /opt/gnucash/include /usr/lib/x86_64-linux-gnu/glib-2.0/include)
link_directories(/opt/gnucash/lib)
link_directories(/opt/gnucash/lib/gnucash)
add_executable(a-gnucash-integration a-gnucash-integration.cpp)
target_link_libraries(a-gnucash-integration gnc-engine gncmod-backend-dbi glib-2.0)
