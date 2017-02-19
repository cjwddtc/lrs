#include "db_mgr.h"
#include <boost/filesystem.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <memory>
#include <sstream>
namespace lsy
{
    db_manager::db_manager(std::string file)
    {
        boost::property_tree::ptree pt;
        boost::property_tree::read_json(file, pt);
        for (auto& a : pt)
        {
            std::string dbfile = a.first;
            dbfile += ".db";
            // checkout if database exist
            bool flag = boost::filesystem::exists(dbfile);
            // connect database
            auto pro_it = emplace((std::string)a.first, tmp::database(dbfile));
            assert(pro_it.second);
            // if database not exist create the table
            if (!flag)
            {
                for (auto& c : a.second.find("table")->second)
                {
                    std::stringstream create_stream;
                    create_stream << "create table " << c.first << " (";
                    bool flag = false;
                    for (auto& b : c.second)
                    {
                        if (flag)
                        {
                            create_stream << ",";
                        }
                        else
                        {
                            flag = true;
                        }
                        create_stream << b.first << " " << b.second.data();
                    }
                    create_stream << ");";
                    auto st = pro_it.first->second.new_statement(
                        create_stream.str());
                    st->OnData.connect([st](bool flag) {
                        assert(flag);
                        st->close();
                    });
                    st->bind();
                }
            }
            for (auto& c : a.second.find("statement")->second)
            {
                for (auto& b : c.second)
                {
                    pro_it.first->second.emplace(
                        b.first, database::statement(&(pro_it.first->second),
                                                     b.second.data()));
                }
            }
        }
    }
    tmp::database& db_manager::operator[](const std::string& str)
    {
        auto it = find(str);
        assert(it != end());
        return it->second;
    }
    namespace tmp
    {
        database::database(database&& other)
            : lsy::database(std::move(other))
        {
            assert(empty());
        }
        database::database(const std::string& str)
            : lsy::database(str)
        {
        }
    }
}