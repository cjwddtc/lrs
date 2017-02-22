#include <boost/foreach.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include <list>
#include <map>
#include <sqlite3.h>
#include <sstream>
#include <string.h>

int main(int argv, char* args[])
{
    boost::property_tree::ptree pt;
    boost::property_tree::read_json(args[2], pt);
    if (strcmp(args[1], "-h") == 0)
        for (auto& a : pt)
        {
            std::cout << "class " << name << "\n";
            std::cout << ",public database \n{\n";
            std::cout << "public:\n";
            std::cout << name << ";\n";
            for (auto& c : a.second.find("statement")->second)
            {
                std::cout << "statement " << c.first << ";\n";
            }
            std::cout << "};\n";
        }
    else if (strcmp(args[1], "-cpp") == 0)
        for (auto& a : pt)
        {
            std::cout << name << "::" << name << "()\n";
            std::cout << "database(\"" << name << ".db\")\n{";
            std::cout << "	if(expire)\n{\n";
            std::cout << "		"

                std::cout
                      << "class " << name << ":public lsy::database \n{\n";
            std::cout << "public:\n";
            std::cout << name << ";";
            std::cout << "" std::string dbfile = a.first;
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
                pro_it.first->second.emplace(
                    c.first, database::statement(&(pro_it.first->second),
                                                 c.second.data()));
            }
        }
    return 0;
}