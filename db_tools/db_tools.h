#pragma once
#include "database.h"
#include <string>

class is_expire
{
  protected:
    bool expire;
    is_expire(const std::string& base, const std::string& derive);
}
