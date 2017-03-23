#include "db_tools.h"
is_expire::is_expire(const std::string& base, const std::string& derive)
    : expire(last_write_time(base) > last_write_time(derive))
{
}