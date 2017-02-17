#include "db_mgr.h"
#include <boost/filesystem.hpp>
#include <boost/property_tree/xml_parser.hpp>
db_manager::db_manager(std::string file)
{
    boost::property_tree::ptree pt;
    boost::property_tree::read_xml(file, pt);
    for (auto p : pt.find("database")->second)
    {
        std::string filename = p.second.get< std::string >("file_name");
        bool        flag     = boost::filesystem::exists(filename);
        dbs.emplace((std::string)p.first, lsy::database(filename));
        if (flag)
        {
            std::string filename = p.second.get< std::string >("json_file");
        }
    }
}

lsy::database& db_manager::operator[](const std::string& str)
{
    assert(dbs.find(str) != dbs.end());
    return dbs.find(str)->second;
}