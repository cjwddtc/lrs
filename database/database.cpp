#include "database.h"
#include <algorithm>
#include <iostream>
namespace lsy
{
	using boost::thread;
	const_blob::const_blob(const void* ptr_, size_t size_)
		: ptr(ptr_)
		, size(size_)
	{
	}


	run_sql_fail_exception::run_sql_fail_exception(std::string str_)
		: str(str_)
	{
	}

	database::database(database&& other)
		: db(other.db)
		, work(io)
	{
		other.db = nullptr;
		other.work.~work();
		other.thr.join();
		thread([this]() { io.run(); }).swap(thr);
	}
	void database::join()
	{
		thr.join();
	}
	void database::statement::bind_(const std::string& str, int n)
	{
		char* ptr = (char*)malloc(str.size() + 1);
		strcpy(ptr, str.c_str());
		assert(sqlite3_bind_text(st, n, ptr, -1, free) == SQLITE_OK);
	}

	void database::statement::bind_(const_blob& m, int n)
	{
		void* nptr = malloc(m.size);
		memcpy(nptr, m.ptr, m.size);
		assert(sqlite3_bind_blob(st, n, nptr, m.size, free) == SQLITE_OK);
	}

	void database::statement::bind_(int m, int n)
	{
		assert(sqlite3_bind_int(st, n, m) == SQLITE_OK);
	}

	void database::statement::bind_(double m, int n)
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
		if (st)
		{
			sqlite3_finalize(st);
		}
	}
	database::statement::statement(statement&& other)
		: st(other.st)
		, io(other.io)
	{
		other.st = nullptr;
	}

	bool database::statement::run()
	{
		switch (sqlite3_step(st))
		{
		case SQLITE_BUSY:
			throw unexpect_busy();
			break;
		case SQLITE_ROW:
			return true;
		case SQLITE_DONE:
			return false;
		default:
			// I know it is the wrong using just change later
			throw(run_sql_fail_exception(
				std::string(sqlite3_errmsg(sqlite3_db_handle(st)))));
		}
	}

	database::statement::statement(database* db, std::string sql)
		: io(db->io)
	{
		io.post([this, db, sql]() {
			assert(sqlite3_prepare_v2(db->db, sql.c_str(), -1, &st, nullptr) == SQLITE_OK); });
	}

	void database::statement::close()
	{
		io.post([this]() { delete this; });
	}

	database::statement::proxy database::statement::operator[](int n)
	{
		return database::statement::proxy(sqlite3_column_value(st, n));
	}

	database::statement::proxy::proxy(sqlite3_value* value_)
		: value(value_)
	{
	}

	database::statement::proxy::operator std::string()
	{
		return std::string((const char*)sqlite3_value_text(value));
	}

	database::statement::proxy::operator const_blob()

	{
		return const_blob(sqlite3_value_blob(value),
			sqlite3_value_bytes(value));
	}

	database::statement::proxy::operator int()
	{
		return sqlite3_value_int(value);
	}

	database::statement::proxy::operator double()
	{
		return sqlite3_value_double(value);
	}

	database::database(std::string path)
		: work(io)
	{
		sqlite3_open(path.c_str(), &db);
		thread([this]() { io.run(); }).swap(thr);
		auto p = new database::statement(this, "PRAGMA synchronous=OFF");
		p->bind();
		p->OnData.connect([p](bool flag) {
			assert(flag);
			p->close();
		});
	}

	database::~database()
	{
		if (db)
		{
			stop();
			sqlite3_close(db);
		}
	}

	database::statement* database::new_statement(std::string sql)
	{
		return new database::statement(this, sql);
	}

	boost::asio::io_service& database::get_io_service()
	{
		return io;
	}

	void database::stop()
	{
		work.~work();
		thr.join();
	}
	sqlite3* database::get_db()
	{
		return db;
	}
}