#include <boost/foreach.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include <list>
#include <map>
#include <sqlite3.h>
#include <sstream>
#include <string.h>

std::string wrap_type(const std::string& type_name,
                      const std::string& value_template)
{
    std::string str(value_template);
    if (type_name == "int")
    {
        str += "<int> ";
    }
    else if (type_name == "double")
    {
        str += "<double> ";
    }
    else if (type_name == "string")
    {
        str += "<std::string> ";
    }
    else if (type_name == "blob")
    {
        str += "<blob> ";
    }
    else
    {
        std::cerr << "unrecognize type:" << type_name << std::endl;
    }
    return str;
}

typedef std::pair< std::string, std::string > value;

int main(int argv, char* args[])
{
    boost::property_tree::ptree pt;
    boost::property_tree::read_json(args[2], pt);
    std::list< std::pair< std::string, std::list< value > > > vec;
    for (auto& a : pt)
    {
        vec.push_back(std::make_pair(a.first, std::list< value >()));
        for (auto& b : a.second)
            vec.back().second.push_back(
                std::make_pair(b.first, b.second.data()));
    }
    if (strcmp(args[1], "-h") == 0)
    {
        std::stringstream steam;
        std::stringstream db_steam;
        steam << "namespace db_class{\n";
        for (auto& a : vec)
        {
            steam << "struct " << a.first << "{\n";
            db_steam << "extern struct "
                     << "{\n";
            for (auto& b : a.second)
            {
                steam << wrap_type(b.second, "value") << b.first << ";\n";
                db_steam << "db_attribute " << b.first << ";\n";
            }
            steam << "};\n";
            db_steam << "}" << a.first << ";\n";
            db_steam << "std::string get_table_name(decltype(\"" << a.first
                     << "\")&);\n";
        }
        steam << "}\n";
        std::cout << "#pragma once" << std::endl;
        std::cout << "#include <string>" << std::endl;
        std::cout << "#include <db_tools.h>" << std::endl;
        std::cout << steam.str() << std::endl;
        std::cout << db_steam.str() << std::endl;
    }
    else if (strcmp(args[1], "-cpp") == 0)
    {
        std::cout << "#include \"" << args[3] << '"' << std::endl;
        for (auto& a : vec)
        {
            std::cout << "decltype(" << a.first << ") " << a.first << "={";
            bool flag = false;
            for (auto& b : a.second)
            {
                if (flag)
                    std::cout << ',';
                std::cout << '.' << b.first << '=' << "db_attribute(\""
                          << a.first << "\",\"" << b.first << "\")";
                flag = true;
            }
            std::cout << "};\n";
            std::cout << "std::string get_table_name(decltype(\"" << a.first
                      << "\")&){\n return \"" << a.first << "\";\n}\n";
        }
    }
    else
    {
        return 1;
    }
    return 0;
}