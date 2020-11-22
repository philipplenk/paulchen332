#ifndef PHILCHESS_DEFAULT_SEARCH_CONTROL_H
#define PHILCHESS_DEFAULT_SEARCH_CONTROL_H

#include <philchess/chessboard.hpp>
#include <philchess/types.hpp>
#include <philchess/zobrist.hpp>

#include <philchess/algorithm/alpha_beta_pruning.hpp>
#include <philchess/algorithm/negamax.hpp>

#include <philchess/eval/see.hpp>

#include <ptl/bit.hpp>
#include <ptl/fixed_capacity_vector.hpp>

#include <algorithm>
#include <atomic>
#include <array>
#include <optional>
#include <vector>

#include <cstdint>

namespace philchess
{
	struct search_parameters
	{
		std::array<std::array<std::uint8_t,32>,32> lmr_depth_movecount=
		{{
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
			{0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,},
			{0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,},
			{0,0,0,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,},
			{0,0,0,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,},
			{0,0,0,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,},
			{0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,},
			{0,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,},
			{0,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,},
			{0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,},
			{0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,},
			{0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,},
			{0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,},
			{0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,},
			{0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,},
			{0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,},
			{0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,},
			{0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,},
			{0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,},
			{0,1,1,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,},
			{0,1,1,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,},
			{0,1,1,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,},
			{0,1,1,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,},
			{0,1,1,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,},
			{0,1,1,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,},
			{0,1,1,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,},
			{0,1,1,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,},
			{0,1,1,2,2,2,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,},
			{0,2,2,2,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,},
			{0,2,2,2,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,},
			{0,2,2,2,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,}
		}};
		
		std::array<std::uint8_t,32> nullmove_depth
		{{
			0,0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,4,4,
		}};
		
		int qs_delta_margin = 1556;
		
		std::array<int,4> futility_margins
		{{
			0,122,227,382
		}};
		
		std::array<int,4> reverse_futility_margins
		{{
			0,109,598,1103
		}};
		
		std::array<int,5> razor_margins
		{{
			0,704,1144,1228,1464
		}};
		
		std::array<int,5> reverse_razor_margins
		{{
			0,283,434,922,2203
		}};
	};
	
	class default_search_control
	{
		public:
		using score_value_type=int;
		
		default_search_control() noexcept = default;
		
		default_search_control(const search_parameters& parameters) noexcept:
			default_search_control()
		{
			parameters_=parameters;
		}
		
		struct eval_t { int eval; score_type type; move m; };
		void cache_eval(const chessboard& board, int eval, score_type type, move m, std::uint8_t depth) noexcept;
		std::optional<eval_t> cached_eval(const chessboard& board, std::uint8_t min_depth) const noexcept;
		
		static int extend_mate_distance(int eval, int added_depth) noexcept
		{
			const auto dist=mate_distance(eval);
			if(dist)
				eval+=eval<0?+added_depth:-added_depth;
			return eval;
		}
		
		void init_search(unsigned depth) noexcept
		{
			for(auto& pv:quadratic_pv_)
				pv.clear();
			
			history_counters_={};
			killers_={};
		}
		
		void init_branch(const chessboard& board, unsigned leftover_depth, unsigned desired_depth) noexcept
		{
			const auto depth=desired_depth-leftover_depth;
			
			moves_tried_[depth]=0;
			static_evals_[depth]=static_eval(board); 		
		}
		
		template <typename DECISION_FUN_T>
		auto list_moves(const chessboard& board, DECISION_FUN_T decision_fun, unsigned desired_depth, unsigned leftover_depth) const noexcept
		{
			auto depth=desired_depth-leftover_depth;
			
			auto moves=board.list_moves();
			if(moves.size()>1)
				order_moves(moves,board,depth);
			
			return moves;
		}
		
		template <typename MOVELIST_T>
		void order_moves(MOVELIST_T& movelist, const chessboard& board, unsigned depth) const noexcept
		{
			const auto cached=cached_eval(board, 0);
		
			auto sort_begin=std::begin(movelist);
			if(cached)
			{
				const auto maybe_best=cached->m;
				for(auto it=sort_begin;it!=std::end(movelist);++it)
				{
					auto& m=*it;
					if(m==maybe_best)
					{
						std::swap(*sort_begin,m);
						++sort_begin;
						break;
					}
				}
			}
			
			if(sort_begin!=std::end(movelist))
			{
				const auto capture_end=std::partition(sort_begin,std::end(movelist),[&](auto m)
				{
					return board.piece_type_at(m.to())!=piece_type::none && eval::see_gain(board,m);
				});
				
				std::sort(sort_begin,capture_end,[&](auto lhs, auto rhs)
				{		
					const static auto& piece_square_tables=eval::get_default_piece_square_table();

					const auto lhs_aggressor=piece_square_tables[board.to_move_][board.piece_type_at(lhs.from())][lhs.from()];
					const auto rhs_aggressor=piece_square_tables[board.to_move_][board.piece_type_at(rhs.from())][rhs.from()];
					
					const auto lhs_victim=piece_square_tables[reverse(board.to_move_)][board.piece_type_at(lhs.to())][lhs.to()];
					const auto rhs_victim=piece_square_tables[reverse(board.to_move_)][board.piece_type_at(rhs.to())][rhs.to()];
											
					return (lhs_victim>rhs_victim || (lhs_victim==rhs_victim && lhs_aggressor<rhs_aggressor));
				});
				
				auto quiet_begin = capture_end;
				for(auto it=quiet_begin;it!=std::end(movelist);++it)
				{
					auto& m=*it;
					if(m==killers_[depth][0] || m==killers_[depth][1])
					{
						std::swap(*quiet_begin,m);
						++quiet_begin;
					}
				}
				
				std::sort(quiet_begin,std::end(movelist),[&,this](auto lhs, auto rhs)
				{
					if(lhs.type()==move_type::promotion && rhs.type()!=move_type::promotion)
						return true;
					if(rhs.type()==move_type::promotion && lhs.type()!=move_type::promotion)
						return false;
					
					if(history_counters_[board.piece_type_at(lhs.from())][lhs.to()]>history_counters_[board.piece_type_at(rhs.from())][rhs.to()])
						return true;
					if(history_counters_[board.piece_type_at(lhs.from())][lhs.to()]<history_counters_[board.piece_type_at(rhs.from())][rhs.to()])
						return false;
					return false;
				});
			}
		}
		
		template <typename MOVELIST_T>
		void adjust_depth(unsigned& desired_depth, unsigned& leftover_depth,const chessboard& board, const MOVELIST_T& movelist) const noexcept
		{
			if(board.is_in_check() || movelist.size()==1)
			{
				desired_depth+=1;
				leftover_depth+=1;
			}
			
			if(leftover_depth>1 && movelist.size()>1 && !cached_eval(board, 0)) //Internal iterative reductions!
			{
				desired_depth-=1;
				leftover_depth-=1;
			}
		}
		
		std::optional<int> should_abort_branch(chessboard& board, unsigned leftover_depth, unsigned desired_depth) const noexcept
		{
			++normal_nodes_;
			
			if(desired_depth==leftover_depth) return std::nullopt;
			
			if(board.is_rule_draw() || is_insufficient_material(board))
				return 0;
			
			return std::nullopt;
		}
		
		template <typename DECISION_FUN_T, typename ABORT_FUN_T>
		std::optional<int> prune_branch(chessboard& board, DECISION_FUN_T decision_fun,ABORT_FUN_T abort_fun, unsigned leftover_depth, unsigned desired_depth)
		{	
			const auto depth = desired_depth-leftover_depth;
			const auto is_pv = std::abs(decision_fun.get_score()+decision_fun.get_reversed().get_score())>1;
				
			auto amount_of_nonpawn_material= ptl::popcount(board.piece_bitboard(board.side_to_move(),piece_type::bishop).ranks() | 
				board.piece_bitboard(board.side_to_move(),piece_type::knight).ranks() |
				board.piece_bitboard(board.side_to_move(),piece_type::rook).ranks() |
				board.piece_bitboard(board.side_to_move(),piece_type::queen).ranks());
			
			//Razoring:
			if
			(
				!is_pv && 
				leftover_depth<parameters_.razor_margins.size() &&
				static_evals_[depth]+parameters_.razor_margins[leftover_depth] <= decision_fun.get_score()
			)
			{
				const auto target_score = decision_fun.get_score()-parameters_.razor_margins[leftover_depth]+1;
				auto zero_window=philchess::algorithm::alpha_beta_pruning<int>{target_score-1,target_score};
				const auto qscore = quiescent_search(board,zero_window,depth);
				if(zero_window(qscore)!=philchess::algorithm::search_decision::cutoff)
					return decision_fun.get_score();
			}
			
			if
			(
				amount_of_nonpawn_material>1 &&
				!is_pv && 
				leftover_depth<parameters_.reverse_razor_margins.size() &&
				!board.is_in_check() &&
				static_evals_[depth]-parameters_.reverse_razor_margins[leftover_depth] > decision_fun.get_score()
			)
			{
				return decision_fun.get_score()+1;
			}
			
			
			//Null move pruning:
			const auto lookup_depth = std::min(static_cast<std::size_t>(leftover_depth),parameters_.nullmove_depth.size()-1);
			const auto null_reduction = parameters_.nullmove_depth[lookup_depth];
			if
			(
				null_reduction>0 &&
				amount_of_nonpawn_material>1 &&
				leftover_depth>null_reduction && 
				depth>0 && 
				!is_pv && 
				!board.last_move_was_null() && 
				!board.is_in_check()
				&& static_evals_[depth]>=-decision_fun.get_reversed().get_score()
			)
			{
				auto beta_score=-decision_fun.get_reversed().get_score();
				
				auto zero_window=philchess::algorithm::alpha_beta_pruning<int>{beta_score-1,beta_score};
				
				auto data=board.do_nullmove();
					auto score=-philchess::algorithm::detail::negamax(board,*this, zero_window.get_reversed(), abort_fun, leftover_depth-1-null_reduction, desired_depth-null_reduction);
				board.undo_nullmove(data);
				
				if(zero_window(score)==philchess::algorithm::search_decision::cutoff)
					return beta_score;
			}
			
			return std::nullopt;
		}
		
		template <typename DECISION_FUN_T, typename ABORT_FUN_T>
		std::optional<int> scout(philchess::move m, chessboard& board, DECISION_FUN_T decision_fun,ABORT_FUN_T abort_fun, unsigned leftover_depth, unsigned desired_depth)
		{
			const auto depth = desired_depth-leftover_depth;
			
			quadratic_pv_[depth+1].clear();
			++moves_tried_[depth];
			
			const auto is_pv = std::abs(decision_fun.get_score()+decision_fun.get_reversed().get_score())>1; //!is_zero_window?
			
			const auto have_best_move = !quadratic_pv_[depth].empty();
			
			const auto is_killer = m==killers_[depth][0] || m==killers_[depth][1];
			
			const auto is_see_gain = eval::see_gain(board,m); //somewhat misleading, gain == not loss. so equality is a gain in my book xD
			
			const bool move_is_interesting = 
				is_see_gain &&
				(
					m.type()==move_type::promotion ||
					board.piece_type_at(m.to())!=piece_type::none || 
					board.would_check(m)
				)
			;
			
			//Futility:
			if(
				leftover_depth<parameters_.futility_margins.size() &&
				static_evals_[depth]+parameters_.futility_margins[leftover_depth]<decision_fun.get_score() &&
				!is_pv &&
				!move_is_interesting && !is_killer
			)
				return decision_fun.get_score();
			
			
			if(
				leftover_depth<parameters_.reverse_futility_margins.size() &&
				static_evals_[depth]-parameters_.reverse_futility_margins[leftover_depth]>decision_fun.get_score() &&
				!is_pv &&
				!board.is_in_check() && is_see_gain
			)
				return decision_fun.get_score()+1;
			
			//Late Move Reductions:
			if(
				moves_tried_[depth]>1 && 
				!board.is_in_check() && !move_is_interesting
			)
			{
				const auto lookup_depth=std::min(static_cast<std::size_t>(leftover_depth),parameters_.lmr_depth_movecount.size()-1);
				const auto lookup_move_n=std::min(static_cast<std::size_t>(moves_tried_[depth]),parameters_.lmr_depth_movecount[lookup_depth].size()-1);
				auto reduction=parameters_.lmr_depth_movecount[lookup_depth][lookup_move_n];
				
				if(reduction>0)
				{
					auto zero_window=philchess::algorithm::alpha_beta_pruning<int>{decision_fun.get_score(),decision_fun.get_score()+1};
					auto undo_data=board.do_move(m);
						auto score=-philchess::algorithm::detail::negamax(board,*this, zero_window.get_reversed(), abort_fun, leftover_depth-1-reduction, desired_depth-reduction);
					board.undo_move(undo_data);
					
					if(zero_window(score)!=philchess::algorithm::search_decision::cutoff)
						return score;
				}
			}
			
			//Zero window search, PVS:
			if(
				leftover_depth>2 && //minimum depth where its beneficial is theoretically 3 if i understood correctly...
				have_best_move &&
				is_pv
			)
			{
				auto zero_window=philchess::algorithm::alpha_beta_pruning<int>{decision_fun.get_score(),decision_fun.get_score()+1};
				auto undo_data=board.do_move(m);
					auto score=-philchess::algorithm::detail::negamax(board,*this, zero_window.get_reversed(), abort_fun, leftover_depth-1, desired_depth);
				board.undo_move(undo_data);
				
				if(zero_window(score)==philchess::algorithm::search_decision::cutoff)
					return std::nullopt;
				return score;
			}
			
			return std::nullopt;
		}
		
		void handle_new_best_move(const chessboard& board, philchess::move m, unsigned depth) noexcept
		{
			quadratic_pv_[depth].clear();
			quadratic_pv_[depth].push_back(m);
			std::copy(std::begin(quadratic_pv_[depth+1]),std::end(quadratic_pv_[depth+1]),std::back_inserter(quadratic_pv_[depth]));
			quadratic_pv_[depth+1].clear();
		}
		
		void handle_cutoff_move(const chessboard& board, philchess::move m, unsigned depth) noexcept
		{			
			++history_counters_[board.piece_type_at(m.from())][m.to()];
			
			quadratic_pv_[depth+1].clear();
			
			if(board.piece_type_at(m.to())==piece_type::none)
			{
				killers_[depth][1] = killers_[depth][0];
				killers_[depth][0] = m;
			}
		}
		
		void handle_discarded_move(const chessboard& board, philchess::move m, unsigned depth) noexcept
		{
			quadratic_pv_[depth+1].clear();
		}
		
		const auto& principal_variation() const noexcept
		{
			return quadratic_pv_[0];
		}
		
		
		int quiescent_search(chessboard& board, algorithm::alpha_beta_pruning<int> decision_fun, unsigned depth) noexcept;
		int static_eval(const chessboard& board) const noexcept;
		int mate_eval(const chessboard& board, unsigned depth) const noexcept;
		static std::optional<int> mate_distance(int score) noexcept;
		static bool is_insufficient_material(const chessboard& board) noexcept;
		
		unsigned number_of_statically_evaluated_nodes() const noexcept { return evaluated_node_num_; }
		unsigned number_of_traversed_nodes() const noexcept { return normal_nodes_; }
		unsigned number_of_quiescent_nodes() const noexcept { return quiescent_nodes_; }
		unsigned number_of_cache_hits() const noexcept { return cache_hits_; }
		unsigned max_quiescent_depth() const noexcept { return quiescent_depth_; }
		void reset_stats() noexcept { evaluated_node_num_=0; cache_hits_=0; quiescent_depth_=0; quiescent_nodes_=0; normal_nodes_=0; }
		
		void reset_tt() noexcept
		{
			std::fill(std::begin(cache_),std::end(cache_),eval_data_t{});
		}
		
		void resize_tt(std::size_t max_size_in_mb)
		{
			const auto max_number_of_entries = max_size_in_mb * 1024*1024 /sizeof(eval_data_t);
			const auto power2_number_of_entries = ptl::bit_floor(max_number_of_entries);
			
			cache_.resize(power2_number_of_entries);
			cache_hash_bitsize_ = ptl::bit_width(power2_number_of_entries) - 1;
		}
		
		std::size_t hashsize_bytes() const noexcept { return cache_.size()*sizeof(eval_data_t); }
		
		private:
		search_parameters parameters_{};
		
		using history_counter_t=piece_type_map<square_table<unsigned>>;
		
		history_counter_t history_counters_{};
					
		std::array<int,64> static_evals_;
		std::array<unsigned,64> moves_tried_;
		
		using killer_pair = std::array<move,2>;
		std::array<killer_pair,64> killers_;
		
		
		static constexpr int max_mate_score=200000, min_mate_score=100000;
		
		struct eval_data_t{ philchess::zobrist zobrist_hash{}; int eval=0; move best_or_refutation_move; score_type type; std::uint8_t depth=0; };
		std::vector<eval_data_t> cache_{2*1024*1024};
		std::size_t cache_hash_bitsize_ = 21;
		
		mutable std::atomic<unsigned> evaluated_node_num_{0}, cache_hits_{0}, quiescent_depth_{0}; 
		mutable std::atomic<unsigned> quiescent_nodes_{0}, normal_nodes_{0};
				
		std::array<ptl::fixed_capacity_vector<move,64>,64> quadratic_pv_{};
	};
	
} //end namespace philchess:engine

#endif
