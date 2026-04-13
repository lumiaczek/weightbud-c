cmake_minimum_required(VERSION 3.10.0)
project(weightbud VERSION 0.0.1 LANGUAGES C)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GTK3 REQUIRED IMPORTED_TARGET gtk+-3.0)

add_executable(weightbud src/main.c)

target_link_libraries(weightbud PRIVATE PkgConfig::GTK3)
