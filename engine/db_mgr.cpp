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
            auto it = dbs.emplace((std::string)a.first, db_proxy(dbfile));
            assert(it.second);
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
                    auto st
                        = it.first->second->new_statement(create_stream.str());
                    st->OnData.connect([st](bool flag) {
                        assert(flag);
                        st->close();
                    });
                    st->bind();
                }
            }
            for (auto& c : a.second.find("statement")->second)
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
                auto st = it.first->second->new_statement(create_stream.str());
                st->OnData.connect([st](bool flag) {
                    assert(flag);
                    st->close();
                });
                st->bind();
            }
        }
    }

    db_manager::db_proxy& db_manager::operator[](const std::string& str)
    {
        assert(dbs.find(str) != dbs.end());
        return dbs.find(str)->second;
    }

    db_manager::db_proxy::db_proxy(db_proxy&& other)
        : db(std::move(other.db))
    {
        assert(sts.empty());
    }
    db_manager::db_proxy::db_proxy(const std::string& str)
        : db(str)
    {
    }
    database* db_manager::db_proxy::operator->()
    {
        return &db;
    }
}