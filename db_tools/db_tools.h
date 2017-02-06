#pragma once
#include <sstream>
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



class BOOST_SYMBOL_EXPORT const_blob
{
  public:
    const void* ptr;
    size_t      size;
    const_blob(const void* ptr, size_t size);
};

class BOOST_SYMBOL_EXPORT blob
{
  public:
    void*  ptr;
    size_t size;
    blob(void* ptr, size_t size);
    operator const_blob();
};

class db_statement_block
{
    typedef boost::variant< const_blob, int, double, vstd::string > db_type;

  protected:
    std::list< db_type > db_value_list;
    std::stringstream    stream;
    void add(db_statement_block& db_st_b);
    db_statement_block(const db_statement_block&);
};

class db_statement_table : public db_statement_block
{
    db_statement_table(const db_statement_block&);
};

class db_statement_condition : public db_statement_block
{
    db_statement_condition(const db_statement_block&);
};

class db_statement_assign : public db_statement_block
{
    db_statement_assign(const db_statement_block&);
};

class db_statement_field : public db_statement_block
{
    db_statement_field(const db_statement_block&);
    template < class T >
    db_statement_field operator+(T a)
    {
        db_statement_field db_st_f(*this);
        db_st_f.db_value_list.push_back(db_type(a));
        db_st_f.stream << "+ ?";
        return std::move(db_st_f);
    }
    template < class T >
    db_statement_field operator-(T a)
    {
        db_statement_field db_st_f(*this);
        db_st_f.db_value_list.push_back(db_type(a));
        db_st_f.stream << "- ?";
        return std::move(db_st_f);
    }
    template < class T >
    db_statement_assign operator=(T a)
    {
        db_statement_assign db_st_f(std::ref(*this));
        db_st_f.db_value_list.push_back(db_type(a));
        db_st_f.stream << "= ?";
        return std::move(db_st_f);
    }
    template < class T >
    db_statement_condition operator==(T a)
    {
        db_statement_condition db_st_f(*this);
        db_st_f.db_value_list.push_back(db_type(a));
        db_st_f.stream << "= ?";
        return std::move(db_st_f);
    }
    template < class T >
    db_statement_condition operator>(T a)
    {
        db_statement_condition db_st_f(*this);
        db_st_f.db_value_list.push_back(db_type(a));
        db_st_f.stream << ">	 ?";
        return std::move(db_st_f);
    }
    template < class T >
    db_statement_condition operator<(T a)
    {
        db_statement_condition db_st_f(*this);
        db_st_f.db_value_list.push_back(db_type(a));
        db_st_f.stream << "<	 ?";
        return std::move(db_st_f);
    }
    template < class T >
    db_statement_condition operator<=(T a)
    {
        db_statement_condition db_st_f(*this);
        db_st_f.db_value_list.push_back(db_type(a));
        db_st_f.stream << "<= ?";
        return std::move(db_st_f);
    }
    template < class T >
    db_statement_condition operator>=(T a)
    {
        db_statement_condition db_st_f(*this);
        db_st_f.db_value_list.push_back(db_type(a));
        db_st_f.stream << ">= ?";
        return std::move(db_st_f);
    }
    template < class T >
    db_statement_condition operator!=(T a)
    {
        db_statement_condition db_st_f(*this);
        db_st_f.db_value_list.push_back(db_type(a));
        db_st_f.stream << "<> ?";
        return std::move(db_st_f);
    }
};

class BOOST_SYMBOL_EXPORT db_attribute
{
  public:
    std::string name;
    std::string table_name;
    std::string str();
    db_attribute(std::string name_, std::string table_name_);
    operator db_statement_field();
};

class db_statement_module
{
    db_statement_block db_st_bl;

  protected:
    tempalte< class T, class... ARG > void add_blocks(T& db_st_b, ARG... arg)
    {
        db_st_bl.add(db_st_b);
        db_st_bl.stream << ",";
        db_st_bl.add_blocks< T >(arg...);
    }
    template < class T >
    void add_blocks(T& db_st_b)
    {
        db_st_bl.add(db_st_b);
    }
    template < class... ARG >
    void select(ARG... arg)
    {
        db_st_bl.stream << "select ";
        add_blocks< db_statement_field >(arg...);
    }
    template < class... ARG >
    void from(ARG... arg)
    {
        db_st_bl.stream << "from ";
        add_blocks< db_statement_table >(arg...);
    }
    template < class... ARG >
    void where(ARG... arg)
    {
        db_st_bl.stream << "where ";
        add_blocks< db_statement_condition >(arg...);
    }
}

class BOOST_SYMBOL_EXPORT database
{
    static sqlite3*                      db;
    static std::thread                   thr;
    static boost::asio::io_service       io;
    static boost::asio::io_service::work work;
};

template < class... ARG >
db_statement select(ARG... arg)
{
    db_statement db_st;
    db_st.stream << "select ";
    select_(db_st, arg...);
    return std::move(db_st);
}

template < class db_traits >

template < class T >
struct db_trait
{
};