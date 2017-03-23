#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include <string.h>

int main(int argv, char* args[])
{
    boost::property_tree::ptree pt;
    boost::property_tree::read_json(args[2], pt);
    bool x = strcmp(args[1], "-cpp") == 0;
    bool y = strcmp(args[1], "-h") == 0;
    if (x | y)
    {
        // for h
        std::cout << "#include \"database.h\"";
        std::cout << "class is_expire\n{\n";
        std::cout << "protected:\n";
        std::cout << "bool expire; \n";
        std::cout << "is_expire(const std::string &derive); \n}\n";
        for (auto& a : pt)
        {
            std::string name = a.first + "_class";
            std::cout << "class " << name << "\n";
            std::cout << ":private is_expire \n{\n";
            std::cout << ",public lsy::database \n{\n";
            std::cout << "public:\n";
            std::cout << name << "();\n";
            for (auto& c : a.second.find("statement")->second)
            {
                std::cout << "statement " << c.first << ";\n";
            }
            std::cout << "};\n";
            std::cout << "extern " << name << " " << a.first << ";\n";
        }
    }
    if (x)
    {
        // for cpp
        for (auto& a : pt)
        {
            std::string name = a.first + "_class";
            std::cout << name << "::" << name << "()\n";
            std::cout << ":is_expire(\"" << a.first << ".db\")\n";
            std::cout << ",lsy::database(\"" << a.first << ".db\")\n";
            for (auto& c : a.second.find("statement")->second)
            {
                std::cout << "," << c.first << "(this,\"" << c.second.data()
                          << "\") \n";
            }
            std::cout << "{\n";
            std::cout << "if(expire){\n";
            for (auto& c : a.second.find("table")->second)
            {
                std::cout << "auto p=new_statement(\"DROP TABLE " << a.first
                          << "\");";
                std::cout << "p->OnData.connect([p](bool "
                             "flag){assert(flag);delete this;});";
                std::cout << "p->bind();";
                std::cout << "p=new_statement(\"CREATE TABLE " << a.first
                          << "(";
                bool flag = false;
                for (auto& d : c.second)
                {
                    if (flag)
                        std::cout << ",";
                    flag = true;
                    std::cout << d.first << " " << d.second.data();
                }
                std::cout << "\");";
                std::cout << "p->OnData.connect([p](bool "
                             "flag){assert(flag);delete this;});";
                std::cout << "p->bind();";
            }
            std::cout << "}\n}\n";
            std::cout << name << " " << a.first << ";\n";
        }
    }

    return 0;
}