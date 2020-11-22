#ifndef PHILCHESS_UCI_WRAPPER_IMPL_H
#define PHILCHESS_UCI_WRAPPER_IMPL_H

/*
	This should never be included directly by user code, the only
	reason it is not inside wrapper.h is for readability purposes ;-);
*/

#include <ptl/typelist.hpp>

#include <charconv>
#include <optional>

namespace philchess {
namespace uci
{
	
template <typename ENGINE_T, typename IO_T>
class wrapper<ENGINE_T, IO_T>::engine_io
{
	public:
	struct depth_info
	{
		unsigned depth, selective_depth;
	};
	
	template <typename PV_T>
	struct printable_pv
	{
		PV_T pv;
		
		friend std::ostream& operator<<(std::ostream& out, const printable_pv& pv)
		{
			out<<" pv";
			for(auto m:pv.pv)
				out<<" "<<m;
			return out;
		}
	};
	
	template <typename SCORE_T>
	struct printable_score
	{
		SCORE_T score;
		std::optional<SCORE_T> mate_distance;
		
		friend std::ostream& operator<<(std::ostream& out, const printable_score& score)
		{
			out<<" score ";
			if(score.mate_distance)
				out<<"mate "<<*(score.mate_distance);
			else
				out<<"cp "<<score.score;
			return out;
		}
	};
	
	template <typename... T>
	void debug_message(const T&... args)
	{
		if(debug_==debug_setting::enabled)
			io_.output("info string ", args...);
	}
	
	template <typename SCORE_T, typename PV_T>
	void report_pv(depth_info depth, std::chrono::milliseconds time, unsigned nodes, SCORE_T score, std::optional<SCORE_T> mate_distance, const PV_T& pv)
	{
		printable_pv<PV_T> print_pv{pv};
		printable_score<SCORE_T> print_score{score, mate_distance};
		
		io_.output(
			"info depth ",depth.depth,
			" seldepth ", depth.selective_depth," ",
			print_score,
			" time ",time.count(),
			print_pv,
			" nodes ",nodes
		);
	}
	
	IO_T io_;
	const std::atomic<debug_setting>& debug_;
};

template <typename ENGINE_T, typename IO_T>
template <typename... ARGS>
wrapper<ENGINE_T, IO_T>::wrapper(IO_T io, ARGS&& ...args):
	io_{io},
	engine_{::std::forward<ARGS>(args)...},
	worker_thd_{[this]()
	{
		for(;;)
		{
			auto fun=task_queue_.wait_and_pop();
			if(!fun())
				break;
		}
	}},
	should_stop_{false}
{}

template <typename ENGINE_T, typename IO_T>
wrapper<ENGINE_T, IO_T>::~wrapper()
{
	should_stop_=true;
	while(task_queue_.try_pop());
	task_queue_.push([](){ return false; });
	worker_thd_.join();
}

template <typename ENGINE_T, typename IO_T>
void wrapper<ENGINE_T, IO_T>::uci()
{
	io_.output("id name ", ENGINE_T::name);
	io_.output("id author ", ENGINE_T::authors);
	
	for(const auto& opt: ENGINE_T::option_list)
		io_.output(opt);
	
	io_.output("uciok");
}

template <typename ENGINE_T, typename IO_T>
void wrapper<ENGINE_T, IO_T>::debug(debug_setting setting)
{
	debug_ = setting;
}

template <typename ENGINE_T, typename IO_T>
void wrapper<ENGINE_T, IO_T>::isready()
{
	io_.output("readyok");
}

namespace setoption_detail
{
	template <typename engine_t, typename option_t, std::size_t option_id>
	constexpr auto engine_supports_option(ptl::typelist<engine_t>, ptl::typelist<option_t>, std::integral_constant<std::size_t, option_id> id) -> 
		decltype(std::declval<engine_t>().set_option(id,std::declval<option_t>()),std::true_type{}) { return {}; }
	
	constexpr std::false_type engine_supports_option(...) { return {}; }
}

template <typename ENGINE_T, typename IO_T>
void wrapper<ENGINE_T, IO_T>::setoption(option opt)
{
	io_.output("info string trying to set option '",opt.name,"' to '",opt.value,"'");
	setoption_impl(opt,std::make_index_sequence<ENGINE_T::option_list.size()>{});
}

template <typename ENGINE_T, typename IO_T>
template <std::size_t... idxs>
void wrapper<ENGINE_T, IO_T>::setoption_impl(option opt, std::index_sequence<idxs...>)
{
	const auto try_parse = [&](const auto& value_description)
	{
		using type = std::decay_t<decltype(value_description)>;
				
		if constexpr(is_option<type,option_type::check>)
		{
			if(opt.value=="true")
				return std::optional<bool>(true);
			else if(opt.value=="false")
				return std::optional<bool>(false);
			
			return std::optional<bool>();
		}
		else if constexpr(is_option<type,option_type::spin>)
		{			
			int parsed_value;
			if(auto [p, ec] = std::from_chars(opt.value.data(), opt.value.data()+opt.value.size(), parsed_value); ec == std::errc())
			{
				if(parsed_value<value_description.min || parsed_value>value_description.max)
				{
					io_.error("Value out of bounds ;_;");
					return std::optional<int>{};
				}
				return std::optional<int>{parsed_value};
			}
			else
			{
				io_.error("Expected an int...");
				return std::optional<int>{};
			}
		}
		else if constexpr(is_option<type,option_type::combo>)
		{
			for(const auto& entry:value_description.values)
			{
				if(opt.value==entry)
					return std::optional<std::string>(std::move(opt.value));
			}
			
			io_.error("Combo entry not found ;_; ;_;");
			return std::optional<std::string>{};
		}
		else if constexpr(is_option<type,option_type::button>)
		{
			return std::optional<ptl::typelist<>>{};
		}
		else if constexpr(is_option<type,option_type::string>)
		{
			return std::optional<std::string>(std::move(opt.value));
		}
		else
		{
			struct unnameable{};
			return std::optional<unnameable>{};
		}
	};
	
	const auto find_and_parse_option = [&](auto option_id)
	{
		if(opt.name==ENGINE_T::option_list[option_id].name)
		{
			std::visit([&](const auto& value)
			{
				auto parsed_value = try_parse(value);
				if(!parsed_value)
				{
					io_.error("Option '",opt.name,"' found, but failed to parse its value");
					return;
				}
				
				if constexpr (setoption_detail::engine_supports_option(ptl::typelist<ENGINE_T>{},ptl::typelist<decltype(*parsed_value)>{},option_id))
				{
					task_queue_.push([this,option_id,parsed_value=std::move(parsed_value)]()
					{
						engine_([&](ENGINE_T& engine)
						{
							engine.set_option(option_id,*parsed_value);
						});
						return true;
					});
				}
				else
					io_.error("Option '",opt.name,"' found and parsed, but engine does not yet support it. Please implement set_option(",option_id,"...)");
				
			},ENGINE_T::option_list[option_id].value);
			return true;
		}
		return false;
	};
	
	const auto found = (find_and_parse_option(std::integral_constant<std::size_t,idxs>{}) || ...);
	
	if(!found)
		io_.output("info string unknown option '",opt.name,"'");
}

template <typename ENGINE_T, typename IO_T>
void wrapper<ENGINE_T, IO_T>::do_register(/*args?*/)
{
	io_.error("register is not implemented yet, as it is not needed for my engine ;-)\n");
}

template <typename ENGINE_T, typename IO_T>
void wrapper<ENGINE_T, IO_T>::ucinewgame()
{
	should_stop_=true;
	task_queue_.push([this]()
	{
		engine_([](ENGINE_T& engine)
		{
			engine.reset();
		});
		return true;
	});
}

template <typename ENGINE_T, typename IO_T>
void wrapper<ENGINE_T, IO_T>::position(board_position pos)
{
	should_stop_=true;
	task_queue_.push([this,pos=::std::move(pos)]()
	{
		engine_([&](ENGINE_T& engine)
		{
			engine.setup(pos.fen);
			for(const auto& m:pos.moves)
				engine.do_move(m);
		});
		return true;
	});
}

template <typename ENGINE_T, typename IO_T>
void wrapper<ENGINE_T, IO_T>::go(search_settings settings)
{
	if(debug_==debug_setting::enabled)
		io_.output("info string starting search with: ",settings);

	should_stop_=false;
	
	task_queue_.push([this, settings]()
	{	
		struct control_t
		{
			const std::atomic<bool>& should_stop;
			engine_io io;
		} control{should_stop_,{io_,debug_}};
		
		auto move=engine_([&control, &settings](ENGINE_T& engine)
		{
			return engine.search(control, settings);
		});
		io_.output("bestmove ", move);
		return true;
	});
}

template <typename ENGINE_T, typename IO_T>
void wrapper<ENGINE_T, IO_T>::stop()
{
	should_stop_=true;
}

template <typename ENGINE_T, typename IO_T>
void wrapper<ENGINE_T, IO_T>::ponderhit()
{
	io_.error("ponderhit is not implemented yet, as it is not needed for my engine ;-)\n");
}

}} //end namespace philchess::uci
#endif
