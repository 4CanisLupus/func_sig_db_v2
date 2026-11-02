cmake_minimum_required(VERSION 3.10)
project(func_sig_db_v2 VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MINGW)
    # Установщик PostgreSQL для Windows содержит libpq, собранную MSVC — она не линкуется
    # с MinGW. Используем libpq из MSYS2 (pacman -S mingw-w64-x86_64-postgresql).
    set(PG_ROOT "C:/msys64/mingw64" CACHE PATH "Корень MinGW с libpq")
    find_path(PostgreSQL_INCLUDE_DIRS libpq-fe.h PATHS ${PG_ROOT}/include NO_DEFAULT_PATH REQUIRED)
    find_library(PostgreSQL_LIBRARIES pq PATHS ${PG_ROOT}/lib NO_DEFAULT_PATH REQUIRED)
else()
    find_package(PostgreSQL REQUIRED)
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(func_sig_db
    src/main.cpp
    src/db_connector.cpp
    src/db_writer.cpp
    src/config_reader.cpp
)

target_link_libraries(func_sig_db PRIVATE ${PostgreSQL_LIBRARIES})

target_include_directories(func_sig_db PRIVATE ${PostgreSQL_INCLUDE_DIRS})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(MSVC)
        target_compile_options(func_sig_db PRIVATE /W4)
    else()
        target_compile_options(func_sig_db PRIVATE -g -Wall -Wextra)
    endif()
endif()

# Копируем конфиг рядом с исполняемым файлом
add_custom_command(TARGET func_sig_db POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/func_sig_db_v2.cfg $<TARGET_FILE_DIR:func_sig_db>/func_sig_db_v2.cfg)
