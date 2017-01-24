#pragma once
template < class T >
class value
{
  public:
    T    val[2];
    bool is_change;
    operator const T&()
    {
        return val[0];
    }
    value& operator=(T value_)
    {
        val[0]    = value_;
        is_change = true;
    }
    void set(T value_)
    {
        val[0]    = value_;
        val[1]    = value_;
        is_change = false;
    }
};

class db_attribute
{
  public:
	std::string name;
	std::string table_name;
	db_attribute(std::string name_,std::string table_name_):name(name_),table_name(table_name_){}
};