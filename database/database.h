#pragma once
#include "database_fwd.h"
#include <iostream>
namespace lsy
{
    template < size_t n, class T, class... ARG >
    void database::statement::bind(T a, ARG... arg)
    {
        if (n == 1)
        {
            reset();
        }

        bind_(a, n);
        bind< n + 1 >(arg...);
    }
    template < size_t n >
    void              database::statement::bind()
    {
        io.post([this]() {
            try
            {
                while (run())
                    OnData(false);
                OnData(true);
            }
            catch (run_sql_fail_exception ec)
            {
                std::cerr << "run sql error:" << ec.str << std::endl;
            }
            catch (unexpect_busy)
            {
                std::cerr << "unexpect_busy please make sure db is only onw by "
                             "the program"
                          << std::endl;
            }
        });
    }
}