cmake_minimum_required(VERSION 3.10.0)
project(weightbud VERSION 0.0.1 LANGUAGES C)

find_package(PkgConfig REQUIRED)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

pkg_check_modules(GTK3 REQUIRED IMPORTED_TARGET gtk+-3.0)
pkg_check_modules(FONTCONFIG REQUIRED IMPORTED_TARGET fontconfig)
pkg_check_modules(SQLITE3 REQUIRED IMPORTED_TARGET sqlite3)

add_executable(weightbud src/main.c src/data_managment.c src/events.c src/storage.c src/ui/form/form.c src/db.c src/ui/dashboard/dashboard.c)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/ui/form/form.ui ${CMAKE_CURRENT_BINARY_DIR}/form.ui COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/ui/dashboard/dashboard.ui ${CMAKE_CURRENT_BINARY_DIR}/dashboard.ui COPYONLY)

target_include_directories(weightbud PRIVATE include)


target_link_libraries(weightbud PRIVATE PkgConfig::GTK3 PkgConfig::FONTCONFIG PkgConfig::SQLITE3)

add_custom_command(TARGET weightbud POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        ${CMAKE_CURRENT_BINARY_DIR}/assets
)