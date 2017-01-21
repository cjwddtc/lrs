add_executable(sqlite shell.c)
add_library(libsqlite STATIC sqlite3.c)
target_link_libraries(sqlite libsqlite)