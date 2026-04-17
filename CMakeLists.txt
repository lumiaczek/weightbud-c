cmake_minimum_required(VERSION 3.10.0)
project(weightbud VERSION 0.0.1 LANGUAGES C)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GTK3 REQUIRED IMPORTED_TARGET gtk+-3.0)

add_executable(weightbud src/main.c src/gui.c)

target_include_directories(weightbud PRIVATE include)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

target_link_libraries(weightbud PRIVATE PkgConfig::GTK3)

add_custom_command(TARGET weightbud POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        ${CMAKE_CURRENT_BINARY_DIR}/assets
)