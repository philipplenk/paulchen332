#include <philchess/bitboard.hpp>
#include <philchess/bitboard_patterns.hpp>
#include <philchess/bitboard_range.hpp>
#include <philchess/chessboard.hpp>
#include <philchess/types.hpp>

#include <philchess/uci/cli.hpp>
#include <philchess/uci/wrapper.hpp>

#include <ptl/bit.hpp>

#include <algorithm>
#include <iostream>
#include <chrono>
#include <limits>
#include <mutex>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>

#include <cmath>

/**
 * Likely the least efficient implementation of TuroChamp possible, just for fun.
 * Rather straightforward and ugly transformation of the program famously described here: https://docs.google.com/file/d/0B0xb4crOvCgTNmEtRXFBQUIxQWs/
 * 
 * Turings Paper is not entirely unambiguous in all instances. In particular:
 * - It seems somewhat unclear to me when he defines a piece to be defended or twice defended. As taking into account only legal moves for defense yields a different third move as in the transcribed game, I took the liberty of interpreting this as attacked by two own pieces, disregarding pins.
 * - I am not entirely certain I understood the rules he applies to evaluate castling rights. I interpreted them as follows: If white still retains castling rights or alternatively has already castled in the current line of search, position play value is increased by 1. Additionally, if the currently legal moves contain a castle, it is increased by another +1.
 * - What exactly is a threat of mate and when is it applicable? I decided on giving this bonus of +1 if the search terminates with black to play, but if black simply passed, white could checkmate.
 * 
 * Interestingly this version produces almost the same number of deviations as the chessbase one(https://en.chessbase.com/post/reconstructing-turing-s-paper-machine), but different ones, so I guess I might have misunderstood something fundamental.
**/

using namespace philchess;
using namespace std::string_view_literals;

namespace
{

class stdio
{
	public:
	
	std::string input()
	{
		std::lock_guard<::std::mutex> lock{input_mutex};
		
		static std::string str;
		std::getline(std::cin,str);
		return str;
	}
	
	template <typename... T>
	void output(T&&... values)
	{
		::std::lock_guard<::std::mutex> lock{output_mutex};
		
		auto unused={((::std::cout<<::std::forward<T>(values)),true)...};
		(void)unused;
		::std::cout<<::std::endl;
	}
	
	template <typename... T>
	void error(T&&... values)
	{
		::std::lock_guard<::std::mutex> lock{output_mutex};
		
		::std::cerr<<"info string ";
		
		auto unused={((::std::cerr<<::std::forward<T>(values)),true)...};
		(void)unused;
		::std::cerr<<::std::endl;
	}
	
	private:
	static ::std::mutex input_mutex, output_mutex;
};

::std::mutex stdio::input_mutex, stdio::output_mutex;

}

namespace turochamp
{
	struct score_type
	{
		double material;
		double position;
	};
	
	constexpr score_type min_score{std::numeric_limits<double>::lowest(),std::numeric_limits<double>::lowest()};
	constexpr score_type max_score{std::numeric_limits<double>::max(),std::numeric_limits<double>::max()};
	
	constexpr bool operator<(score_type lhs, score_type rhs) noexcept
	{
		return std::tie(lhs.material,lhs.position)<std::tie(rhs.material,rhs.position);
	}
	
	constexpr bool operator>(score_type lhs, score_type rhs) noexcept
	{
		return std::tie(lhs.material,lhs.position)>std::tie(rhs.material,rhs.position);
	}
	
	constexpr bool operator==(score_type lhs, score_type rhs) noexcept
	{
		return std::tie(lhs.material,lhs.position)==std::tie(rhs.material,rhs.position);
	}
	
	constexpr auto piece_values = []()
	{
		piece_type_map<double> values;
		values[piece_type::none]=0;
		values[piece_type::king]=0;
		values[piece_type::pawn]=1;
		values[piece_type::knight]=3;
		values[piece_type::bishop]=3.5;
		values[piece_type::rook]=5;
		values[piece_type::queen]=10;
		
		return values;
	}();
	
	struct history_info
	{
		move last_move;
		bool was_capture = false;
		bool has_castled = false;
	};
	
	double rounded_sqrt(double v) noexcept
	{
		double ret_val=0;
		std::modf((std::sqrt(v)+0.05)*10,&ret_val);
		return ret_val/10;
	}
	
	struct occupancy_pair
	{
		bitboard::all all;
		side_map<bitboard::rank> per_side;
	};
	
	occupancy_pair compute_occupancy(const chessboard& board) noexcept
	{
		occupancy_pair ret_val;
		
		for(unsigned i=0;i<64;++i)
		{
			const square sq{i};
			const auto type = board.piece_type_at(sq);
			const auto owner = board.owner_at(sq);
			
			if(type!=piece_type::none)
			{
				ret_val.all.set(sq);
				ret_val.per_side[owner].set(sq);
			}
		}
		
		return ret_val;
	}
	
	auto compute_defense_maps(const chessboard& board, const occupancy_pair& occupancy, side active_side) noexcept
	{
		struct ret_val_t
		{
			square_table<int> count, piece_count;
		} ret_val;
		
		for(unsigned i=0;i<64;++i)
		{
			const square sq{i};
			const auto type = board.piece_type_at(sq);
			const auto owner = board.owner_at(sq);
			
			if(type!=piece_type::none && owner==active_side)
			{
				for(auto target: squares(attacked_by(active_side,type,sq,occupancy.all)))
				{
					++ret_val.count[target];
					if(type!=piece_type::pawn)
						++ret_val.piece_count[target];
				}
			}
		}
		
		return ret_val;
	}
	
	bool is_defended(const chessboard& board, square target_sq) noexcept
	{
		const auto occupancy = compute_occupancy(board);
	
		for(unsigned i=0;i<64;++i)
		{
			const square sq{i};
			const auto type = board.piece_type_at(sq);
			const auto owner = board.owner_at(sq);
			
			if(type!=piece_type::none && owner==reverse(board.side_to_move()))
			{
				for(auto target: squares(attacked_by(reverse(board.side_to_move()),type,sq,occupancy.all)))
				{
					if(target==target_sq)
						return true;
				}
			}
		}
		return false;
	}
	
	bool is_considerable(chessboard& board, history_info history, move m, int depth) noexcept
	{
		if(depth>0)
			return true;
		
		auto undo_data = board.do_move(m);
			auto replies = board.list_moves();
		board.undo_move(undo_data);
		
		if(board.piece_type_at(m.to())!=piece_type::none) //is capture and
		{
			if(piece_values[board.piece_type_at(m.to())]>piece_values[board.piece_type_at(m.from())]) //higher value than capturing piece
				return true;

			if(history.was_capture && history.last_move.to()==m.to()) //recapture
				return true;
		
			if(!is_defended(board,m.to()))
				return true;
		}
		
		//gives checkmate
		if(board.would_check(m) && replies.empty())
			return true;
		
		return false;
	}
	
	score_type evaluate(chessboard& board, side active_side, history_info history) noexcept
	{
		using movelist_type = std::decay_t<decltype(board.list_moves())>;
		
		score_type ret_val{0,0};
		
		side_map<movelist_type> movelists;
		movelists[board.side_to_move()]=board.list_moves();
		movelists[reverse(board.side_to_move())]=[&]()
		{
			auto undo_data = board.do_nullmove();
				auto result = board.list_moves();
			board.undo_nullmove(undo_data);
			
			return result;
		}();
		
		bool could_castle = false;
		square_table<unsigned> move_counts{};
		for(auto m:movelists[active_side])
		{
			if(m.type()!=move_type::castling)
			{
				++move_counts[m.from()];
				if(board.piece_type_at(m.to())!=piece_type::none)
					++move_counts[m.from()];
			}
			else
				could_castle = true;
		}
		
		if(could_castle || history.has_castled)
			ret_val.position+=1;
			
		if(board.castling_rights()[active_side]!=castling_right::none)
			ret_val.position+=1;
		
		const auto occupancy = compute_occupancy(board);
		const auto defenders = compute_defense_maps(board,occupancy,active_side);

		square king_square;
				
		side_map<double> material_sums{0,0};
		for(unsigned i=0;i<64;++i)
		{
			const square sq{i};
			const auto type = board.piece_type_at(sq);
			const auto owner = board.owner_at(sq);
			
			material_sums[owner]+=piece_values[type];
			
			if(owner==active_side)
			{
				switch(type)
				{
					case piece_type::king:
						king_square = sq;
						ret_val.position+=rounded_sqrt(move_counts[sq]);
						break;
					case piece_type::bishop:
					case piece_type::knight:
					case piece_type::rook:
						if(defenders.count[sq]>0)
							ret_val.position+=1;
						if(defenders.count[sq]>1)
							ret_val.position+=0.5;
							
						[[fallthrough]];
					case piece_type::queen:
						ret_val.position+=rounded_sqrt(move_counts[sq]);
						break;
					case piece_type::pawn:
					{
						const auto distance = active_side==side::white?(sq.rank()-1):(6-sq.rank());
						ret_val.position+=0.2*distance;
						if(defenders.piece_count[sq]>0)
							ret_val.position+=0.3;
						break;
					}
					default:
						break;
				}
			}
		}
		
		const auto pseudo_queen_targets = attacked_by_queen(king_square,occupancy.all);
		const auto pseudo_queen_normal = ptl::popcount((pseudo_queen_targets&(~occupancy.all)).ranks());
		const auto pseudo_queen_capture = ptl::popcount((pseudo_queen_targets&occupancy.per_side[reverse(active_side)]).ranks());
		ret_val.position-=rounded_sqrt(pseudo_queen_normal+2*pseudo_queen_capture);
		
		if(movelists[board.side_to_move()].empty())
			material_sums[reverse(board.side_to_move())]+=1000;

		if(board.side_to_move()==reverse(active_side) && board.is_in_check())
			ret_val.position+=0.5;
			
		if(board.side_to_move()!=active_side)
		{
			bool could_mate = false;
			auto undo_data = board.do_nullmove();
				auto potential_moves = board.list_moves();
				for(auto m: potential_moves)
				{
					if(board.would_check(m))
					{
						auto undo_data = board.do_move(m);
						if(board.list_moves().empty())
							could_mate = true;
						board.undo_move(undo_data);
					}
				}
			board.undo_nullmove(undo_data);
			if(could_mate)
				ret_val.position+=1;
		}
		
		ret_val.material=material_sums[active_side]/material_sums[reverse(active_side)];
		
		return ret_val;
	}
	
	score_type minimax_value(chessboard& board, history_info history, side active_side, int depth)
	{
		score_type selected_score = board.side_to_move()==active_side?min_score:max_score;
		const auto is_better = [&](const score_type& new_score)
		{
			return board.side_to_move()==active_side?
				(new_score>selected_score):
				(new_score<selected_score);
		};
		
		unsigned moves_tried = 0;
		
		auto moves = board.list_moves();
		
		for(auto m: moves)
		{
			if(!is_considerable(board,history,m,depth))
				continue;
			
			++moves_tried;
			
			history_info new_history;
			new_history.last_move = m;
			new_history.was_capture = board.piece_type_at(m.to())!=piece_type::none;
			new_history.has_castled = (board.side_to_move()==active_side && m.type()==move_type::castling) || history.has_castled;
			
			auto undo_data = board.do_move(m);
				auto value = minimax_value(board,new_history,active_side,depth-1);
			board.undo_move(undo_data);
			
			if(is_better(value))
				selected_score = value;
		}
		
		if(moves_tried==moves.size())
			return selected_score;
			
		if(board.side_to_move()==active_side)
			return std::max(selected_score,evaluate(board,active_side,history));
		
		return std::min(selected_score,evaluate(board,active_side,history));
	}
	
	class engine
	{
		public:
		
		constexpr static auto name="Turochamp"sv;
		constexpr static auto authors="Alan Turing and David Champernowne(with horrid implementation by Philipp Lenk)"sv;
		const static std::array<uci::option_description,0> option_list;
		
		void reset()
		{
			board.setup("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
		}
		
		void setup(std::string_view fen)
		{
			board.setup(fen);
		}
		
		void do_move(philchess::move m)
		{
			board.do_move(m);
		}

		template <typename SEARCH_CONTROL_T, typename SEARCH_SETTINGS_T>
		auto search(SEARCH_CONTROL_T control, SEARCH_SETTINGS_T settings)
		{
			auto start = std::chrono::high_resolution_clock::now();
			
			const auto moves = board.list_moves();
			const auto active_side = board.side_to_move();
			
			move best_move;
			score_type best_score = min_score;
			
			for(auto m: moves)
			{
				history_info history;
				history.last_move = m;
				history.was_capture = board.piece_type_at(m.to())!=piece_type::none;
				history.has_castled = m.type()==move_type::castling;
				
				auto undo_data = board.do_move(m);
					auto value = minimax_value(board,history,active_side,full_depth-1);
				board.undo_move(undo_data);
				
				control.io.debug_message("move ",m," gives material value of ",value.material," and position value of ",value.position);
				if(value>best_score)
				{
					best_score = value;
					best_move = m;
				}
			}
			
			const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
			control.io.report_pv({full_depth,0},elapsed,0, static_cast<int>(best_score.material*100+best_score.position), std::optional<int>{}, std::array{best_move});
			control.io.debug_message("material: ",best_score.material);
			control.io.debug_message("position: ",best_score.position);
			
			return best_move;
		}

		private:
		chessboard board;
		unsigned full_depth = 2;
	};
	
	constexpr std::string_view engine::name;
	constexpr std::string_view engine::authors;
	const std::array<uci::option_description,0> engine::option_list{};
}

int main(int argc, char* argv[])
{
	using engine_type=philchess::uci::wrapper<turochamp::engine, stdio>;
	philchess::uci::run_cli<engine_type>(stdio{});
	return 0;
}
