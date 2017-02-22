#pragma once
#include <boost/property_tree/ptree.hpp>
#include <string>

class is_expire
{
  protected:
    bool expire;
    is_expire(const std::string& name);
}

class database : private is_expire,
                 public lsy::databasse
{
    database(std::pair< std::string, boost::property_tree::ptree > pt);
}