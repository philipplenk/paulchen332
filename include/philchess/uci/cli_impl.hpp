#ifndef PHILCHESS_CLI_IMPL_H
#define PHILCHESS_CLI_IMPL_H

/*
	This should never be included directly by user code, the only
	reason it is not inside cli.h is for readability purposes ;-);
*/

#include <philchess/uci/wrapper.hpp>

#include <ptl/flatmap.hpp>

#include <algorithm>
#include <sstream>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

namespace philchess {
namespace uci
{
	namespace detail
	{	
		template <auto>
		class call_helper;

		template <typename CLASS_T, typename R, typename... ARG_T, R(CLASS_T::*memfun)(ARG_T...)>
		class call_helper<memfun>
		{
			public:
			static void call(CLASS_T& handler, std::istringstream& data)
			{
				call_impl(handler, data, std::index_sequence_for<ARG_T...>{});
			}
			
			private:
			template <std::size_t ...IDX>
			static void call_impl(CLASS_T& handler, std::istringstream& data, std::index_sequence<IDX...>)
			{
				std::tuple<std::decay_t<ARG_T>...> args;
				((data>>std::get<IDX>(args)),...);
				
				(handler.*memfun)(std::move(std::get<IDX>(args))...);
			}
		};
		
		template <auto v>
		constexpr auto call_helper_v = call_helper<v>::call;
	}
	
	template <typename ENGINE_T, typename IO_T, typename ...ARGS_T>
	void run_cli(IO_T io, ARGS_T&&... args)
	{
		ENGINE_T engine{io, std::forward<ARGS_T>(args)...};
		
		using namespace std::string_view_literals;
		using command_handler_t=std::add_pointer_t<void(ENGINE_T&, std::istringstream&)>;
		
		constexpr auto command_map=ptl::make_fixed_flatmap<std::string_view, command_handler_t>(
		{
			{ "uci"sv,			detail::call_helper_v<&ENGINE_T::uci> },
			{ "debug"sv,		detail::call_helper_v<&ENGINE_T::debug> },
			{ "isready"sv,		detail::call_helper_v<&ENGINE_T::isready> },
			{ "setoption"sv,	detail::call_helper_v<&ENGINE_T::setoption> },
			{ "register"sv,		detail::call_helper_v<&ENGINE_T::do_register> },
			{ "ucinewgame"sv,	detail::call_helper_v<&ENGINE_T::ucinewgame> },
			{ "position"sv,		detail::call_helper_v<&ENGINE_T::position> },
			{ "go"sv,			detail::call_helper_v<&ENGINE_T::go> },
			{ "stop"sv,			detail::call_helper_v<&ENGINE_T::stop> },
			{ "ponderhit"sv,	detail::call_helper_v<&ENGINE_T::ponderhit> },
		});
		
		std::string line, cmd;
		for(;;)
		{
			line=io.input();
			std::istringstream stream(line);
			stream>>cmd;
			
			if(cmd=="quit"sv)
				break;
			
			if(auto it=command_map.find(cmd); it!=command_map.end())
				it->second(engine,stream);
			else
				io.error("Unknown command '",cmd,"' ;_;");
		}
	}
	
}} //end namespace philchess:uci

#endif
