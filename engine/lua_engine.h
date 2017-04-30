extern "C" {
#include <lua.h>
}
#include <boost/variant.hpp>
#include <functional>
#include <vector>
#include <map>

template <class T>
class signal
{
	typedef boost::variant<lua_CFunction, std::function<void(T)>> function;
	class function_run : public boost::static_visitor<T> {
	public:
		void operator()(lua_CFunction func) const {
			std::cout << "It's an int: " << i << '\n';
		}
		void operator()(std::string s) const {
			std::cout << "It's a std::string: " << s << '\n';
		}
	};
	static void run(function func, T &a) {

	}
	std::vector<function> func;
	void resign(lua_CFunction func);
	void resign(std::function<void(T)> func);
	void trigger(T a);
};

std::map<std::string, boost::variant<signal<std::string>, signal<int32_t>>> signals;