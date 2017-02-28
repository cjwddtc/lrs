#pragma once
#include "database_fwd.h"
#include <iostream>
namespace lsy
{

	template <class ...ARG>
	void database::statement::bind(ARG ...arg)
	{
		io.post([this, &arg...]() {
			reset();
			bind_t<1>(arg...);
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
	template < size_t n, class T, class... ARG >
	void database::statement::bind_t(T a, ARG... arg)
	{
		bind_(a, n);
		bind_t< n + 1 >(arg...);
	}
	template < size_t n >
	void              database::statement::bind_t()
	{
	}
}