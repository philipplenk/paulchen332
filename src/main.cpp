#include "engine/paulchen332.hpp"

#include <philchess/uci/cli.hpp>
#include <philchess/uci/wrapper.hpp>

#include <iostream>
#include <mutex>
#include <string>

namespace
{
	class stdio
	{
		public:
		std::string input()
		{
			std::lock_guard<std::mutex> lock{input_mutex};
			
			static std::string str;
			std::getline(std::cin,str);
			return str;
		}
		
		template <typename... T>
		void output(T&&... values)
		{
			std::lock_guard<std::mutex> lock{output_mutex};
			
			((std::cout<<std::forward<T>(values)),...);
			std::cout<<std::endl;
		}
		
		template <typename... T>
		void error(T&&... values)
		{
			std::lock_guard<std::mutex> lock{output_mutex};
			
			std::cerr<<"info string ";
			((std::cerr<<std::forward<T>(values)),...);
			std::cerr<<::std::endl;
		}
		
		private:
		static std::mutex input_mutex, output_mutex;
	};

	std::mutex stdio::input_mutex, stdio::output_mutex;

}

int main(int argc, char* argv[])
{	
	using engine_type=philchess::uci::wrapper<philchess::engine::paulchen332, stdio>;
	philchess::uci::run_cli<engine_type>(stdio{});
	return 0;
}
