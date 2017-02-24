#pragma once
#include <string>
#include "database.h"

class is_expire
{
  protected:
    bool expire;
    is_expire(const std::string& base,const std::string &derive);
}
