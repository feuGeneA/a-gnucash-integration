cmake_minimum_required(VERSION 2.8.11)
project(a-gnucash-integration)
add_definitions(-DHAVE_SCANF_LLD) # to satisfy gnucash headers
include_directories(/usr/include/glib-2.0 /usr/lib/x86_64-linux-gnu/glib-2.0/include)
link_directories(/usr/lib/x86_64-linux-gnu/gnucash)
link_directories(/usr/lib/x86_64-linux-gnu/gnucash/gnucash)
add_executable(a-gnucash-integration a-gnucash-integration.cpp)
target_link_libraries(a-gnucash-integration gnc-engine gncmod-backend-dbi)
