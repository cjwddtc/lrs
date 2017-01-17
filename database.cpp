#include "database.h"
#include <iostream>

const_blob::const_blob(const void* ptr_, size_t size_)
    : ptr(ptr_)
    , size(size_)
{
}

void database::statement::bind_(std::string str, int n)
{
    char* ptr = (char*)malloc(str.size() + 1);
    strcpy(ptr, str.c_str());
    assert(sqlite3_bind_text(st, n, ptr, -1, free) == SQLITE_OK);
}

void database::statement::bind_(blob m, int n)
{
    void* nptr = malloc(m.size);
    memcpy(nptr, m.ptr, m.size);
    assert(sqlite3_bind_blob(st, n, nptr, m.size, free) == SQLITE_OK);
}

void database::statement::bind_(int m, int n)
{
    assert(sqlite3_bind_int(st, n, m) == SQLITE_OK);
}

void database::statement::bind_(float m, int n)
{
    assert(sqlite3_bind_double(st, n, m) == SQLITE_OK);
}

void database::statement::reset()
{
    sqlite3_reset(st);
    sqlite3_clear_bindings(st);
}

database::statement::~statement()
{
    sqlite3_finalize(st);
}

database::statement::statement(database* db, std::string sql)
    : io(db->io)
{
    sqlite3_prepare_v2(db->db, sql.c_str(), -1, &st, 0);
}

void database::statement::bind()
{
}

database::statement::proxy database::statement::operator[](int n)
{
    return database::statement::proxy(sqlite3_column_value(st, n));
}

void database::statement::async_run(std::function<void(void)> func)
{
    io.post([func, this]() {
	switch(sqlite3_step(st))
	    {
	    case SQLITE_BUSY:
		throw unexpect_busy();
		break;
	    case SQLITE_DONE:
		func();
		break;
	    default:
		// I know it is the wrong using just change later
		throw(run_sql_fail_exception(
		    std::string(sqlite3_errmsg(sqlite3_db_handle(st)))));
	    }
    });
}

database::statement::proxy::proxy(sqlite3_value* value_)
    : value(value_)
{
}

database::statement::proxy::operator const char*()
{
    return (const char*)sqlite3_value_text(value);
}

database::statement::proxy::operator const_blob()
{
    return const_blob(sqlite3_value_blob(value), sqlite3_value_bytes(value));
}

database::statement::proxy::operator int()
{
    return sqlite3_value_int(value);
}

template <class T>
bind_base<T>::statement::statement(bind_base<T>& bind_, std::string condition)
    : bind(bind_)
    , database::statement(bind_.db, select + condition)
{
}
template <class T>
void bind_base<T>::statement::async_run(std::function<void(T&)> func)
{
}
