#ifndef PHILCHESS_UCI_TYPES_H
#define PHILCHESS_UCI_TYPES_H

#include <philchess/types.hpp>

#include <chrono>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

namespace philchess {
namespace uci
{
	enum class debug_setting
	{
		enabled,
		disabled
	};
	
	inline std::istream& operator>>(std::istream& in, debug_setting& state)
	{
		std::string value;
		in>>value;
		
		if(value=="on")
			state=debug_setting::enabled;
		else if(value=="off")
			state=debug_setting::disabled;
		else
			in.setstate(std::ios_base::failbit);
		
		return in;
	}
	
	inline ::std::ostream& operator<<(::std::ostream& out, const debug_setting& state)
	{
		if(state==debug_setting::enabled)
			out<<"enabled";
		else
			out<<"disabled";

		return out;
	}
	
	enum class option_type
	{
		check,
		spin,
		combo,
		button,
		string,
		none
	};
	
	template <option_type t>
	struct option_value{};
	
	template <>
	struct option_value<option_type::check> { bool value; };
	
	template <>
	struct option_value<option_type::spin> { int value, min, max; };
	
	template <>
	struct option_value<option_type::combo> { std::vector<std::string> values; };
	
	template <>
	struct option_value<option_type::string> { std::string value; };
	
	template <option_type... ts>
	using option_value_variant = std::variant<option_value<ts>...>;
	using option_type_variant = option_value_variant<option_type::button,option_type::check,option_type::combo, option_type::none, option_type::spin, option_type::string>;
	
	
	template <typename type, option_type t>
	constexpr bool is_option = std::is_same_v<type,option_value<t>>;
	
	struct option_description
	{
		std::string_view name;
		option_type_variant value;
	};
	
	inline ::std::ostream& operator<<(::std::ostream& out, const option_description& opt)
	{
		out<<"option name "<<opt.name;
		std::visit([&](auto& v)
		{
			using type = std::decay_t<decltype(v)>;
			
			if constexpr(is_option<type,option_type::check>)
				out<<" type check default "<<(v.value?"true":"false");
			else if constexpr(is_option<type,option_type::spin>)
				out<<" type spin default "<<v.value<<" min "<<v.min<<" max "<<v.max;
			else if constexpr(is_option<type,option_type::combo>)
			{
				out<<" type combo";
				if(!v.values.empty())
					out<<" default "<<v.values[0];
				for(const auto& entry:v.values)
					out<<" var "<<entry;
			}
			else if constexpr(is_option<type,option_type::button>)
				out<<" type button";
			else if constexpr(is_option<type,option_type::string>)
				out<<" type string default "<<v.value;
		},opt.value);
		
		return out;
	}
	
	struct option
	{
		std::string name, value;
	};
	
	inline std::istream& operator>>(std::istream& in, option& opt)
	{
		std::string line;
		std::getline(in,line);
		
		constexpr std::string_view value_sep=" value ";
		const auto value_pos=line.find(value_sep);
		
		opt.name=line.substr(1,value_pos-1);
		
		if(value_pos!=line.npos)
			opt.value=line.substr(value_pos+value_sep.size());
		
		return in;
	}
	
	inline std::ostream& operator<<(std::ostream& out, const option& opt)
	{
		out<<"Name: '"<<opt.name;
		if(!opt.value.empty())
			out<<", ValueString: '"<<opt.value<<"'";

		return out;
	}
	
	struct board_position
	{
		std::string fen="rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
		std::vector<move> moves;
	};
	
	inline std::istream& operator>>(std::istream& in, board_position& pos)
	{
		std::string value;
		in>>value;
		
		if(value=="startpos")
		{
			pos.fen="rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
		}
		else if(value=="fen")
		{
			in>>value;
			pos.fen=value;
			
			in>>value;
			pos.fen+=' '+value; //side to move
			
			in>>value;
			pos.fen+=' '+value; //castling rights
			
			in>>value;
			pos.fen+=' '+value; //en passant square
			
			in>>value;
			pos.fen+=' '+value; //halfmove clock
			
			in>>value;
			pos.fen+=' '+value; //fullmove number
		}
		else
		{
			in.setstate(std::ios_base::failbit);
			return in;
		}
		
		in>>value;
		
		if(value=="moves")
		{
			philchess::move m;
			while(in>>m)
				pos.moves.push_back(m);
		}
		
		return in;
	}
	
	struct search_settings
	{
		philchess::side_map<std::optional<std::chrono::milliseconds>> remaining_time{};
		philchess::side_map<std::chrono::milliseconds> increment{{{std::chrono::milliseconds{0},std::chrono::milliseconds{0}}}};
		
		std::optional<unsigned> moves_to_go;
		unsigned depth=41;
		
	};
	
	inline std::istream& operator>>(std::istream& in, search_settings& settings) //see above, incomplete...
	{
		std::string value;
		while(in>>value)
		{
			if(value=="wtime")
			{
				unsigned long ms;
				in>>ms;
				settings.remaining_time[side::white]=std::chrono::milliseconds{ms};
			}
			else if(value=="btime")
			{
				unsigned long ms;
				in>>ms;
				settings.remaining_time[side::black]=std::chrono::milliseconds{ms};
			}
			else if(value=="winc")
			{
				unsigned long ms;
				in>>ms;
				settings.increment[side::white]=std::chrono::milliseconds{ms};
			}
			else if(value=="binc")
			{
				unsigned long ms;
				in>>ms;
				settings.increment[side::black]=std::chrono::milliseconds{ms};
			}
			else if(value=="movestogo")
			{
				unsigned to_go;
				in>>to_go;
				settings.moves_to_go=to_go;
			}
			else if(value=="depth")
			{
				unsigned depth;
				in>>depth;
				settings.depth=depth;
			}
			else
			{
				in.setstate(std::ios_base::failbit);
				return in;
			}
		}
		return in;
	}
	
	inline ::std::ostream& operator<<(::std::ostream& out, const search_settings& opt)
	{
		if(opt.remaining_time[side::white])
			out<<"White remaining: "<<opt.remaining_time[side::white]->count()<<"ms ";
		
		if(opt.remaining_time[side::black])
			out<<"Black remaining: "<<opt.remaining_time[side::black]->count()<<"ms ";
			
		out<<"White increment: "<<opt.increment[side::white].count()<<"ms ";
		out<<"Black increment: "<<opt.increment[side::black].count()<<"ms ";
			
		if(opt.moves_to_go)
			out<<*opt.moves_to_go<<" moves to go ";
		out<<opt.depth<<" plies to search";
		
		return out;
	}

}} //end namespace philchess::uci

#endif
