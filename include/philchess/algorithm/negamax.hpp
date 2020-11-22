#ifndef PHILCHESS_ALGORITHM_NEGAMAX_H
#define PHILCHESS_ALGORITHM_NEGAMAX_H

#include <philchess/algorithm/algorithm.hpp>

#include <limits>
#include <optional>
#include <type_traits>

namespace philchess {
namespace algorithm
{

namespace detail
{
	
struct search_aborted_exception{};

template <typename SEARCH_CONTROL_T>
struct negamax_result_t
{
	using pv_t=std::decay_t<decltype(std::declval<SEARCH_CONTROL_T>().principal_variation())>;
	using score_t=typename SEARCH_CONTROL_T::score_value_type;
	
	pv_t pv;
	score_t eval;
};

template <typename BOARD_T, typename SEARCH_CONTROL_T, typename DECISION_FUN_T, typename ABORT_FUN_T>
auto negamax(BOARD_T& board, SEARCH_CONTROL_T& control, DECISION_FUN_T decision_fun, ABORT_FUN_T abort_fun, unsigned leftover_depth, unsigned desired_depth)
{
	using score_t=typename SEARCH_CONTROL_T::score_value_type;
	auto cached_return=[&control, &board, depth=leftover_depth](score_t score, score_type type, move m) { control.cache_eval(board,score,type,m,depth); return score; };
	
	const auto abort_result=control.should_abort_branch(board, leftover_depth, desired_depth);
	if(abort_result)
		return *abort_result;
		
	move best_move;
	
	const auto cached_result=handle_cached_eval(
		control.cached_eval(board,leftover_depth),control,decision_fun,
		[&](const auto& eval){ best_move=eval->m; control.handle_new_best_move(board,eval->m,desired_depth-leftover_depth); },
		desired_depth-leftover_depth
	);
	if(cached_result)
		return *cached_result;
	
	if(leftover_depth==0)
		return control.quiescent_search(board, decision_fun,desired_depth-leftover_depth);
	
	if(abort_fun())
		throw search_aborted_exception{};
		
	control.init_branch(board, leftover_depth, desired_depth);
	
	auto pruning_score=control.prune_branch(board, decision_fun, abort_fun,leftover_depth, desired_depth);
	if(pruning_score)
		return *pruning_score;
		
	auto movelist=control.list_moves(board,decision_fun, desired_depth,leftover_depth);
	if(movelist.empty())
		return control.mate_eval(board, desired_depth-leftover_depth);
	
	control.adjust_depth(desired_depth, leftover_depth, board, movelist);
		
	score_type type=score_type::upper_bound;
	
	for(const auto move: movelist)
	{
		score_t score;
		auto potential_score=control.scout(move,board,decision_fun, abort_fun, leftover_depth, desired_depth);
		if(potential_score)
			score=*potential_score;
		else
		{
			auto undo_data=board.do_move(move);  //The reason I used something that has to be undone manually(instead of some RAII wrapper) is the simple fact that not fixing the board(copy) in case of an exception is a rather useful optimization... 
				score=-negamax(board,control, decision_fun.get_reversed(), abort_fun, leftover_depth-1, desired_depth);
			board.undo_move(undo_data);
		}
		
		switch(decision_fun(score))
		{
			case search_decision::cutoff:
			{
				control.handle_cutoff_move(board, move,desired_depth-leftover_depth);
				return cached_return(decision_fun.get_score(),score_type::lower_bound, move);
			}
			case search_decision::store_and_continue:
			{
				control.handle_new_best_move(board, move,desired_depth-leftover_depth);
				type=score_type::exact;
				best_move=move;
				break;
			}
			case search_decision::continue_search:
			{
				control.handle_discarded_move(board,move,desired_depth-leftover_depth);
				break;
			}
		}
	}
	
	return cached_return(decision_fun.get_score(),type, best_move);
}

} //end namespace detail

template <typename BOARD_T, typename SEARCH_CONTROL_T, typename DECISION_FUN_T, typename ABORT_FUN_T>
auto negamax(BOARD_T board, SEARCH_CONTROL_T& control, DECISION_FUN_T decision_fun, ABORT_FUN_T abort_fun, unsigned desired_depth) noexcept
{
	using result_t=detail::negamax_result_t<SEARCH_CONTROL_T>;
	
	try
	{
		control.init_search(desired_depth);
		auto score=detail::negamax(board,control,decision_fun, abort_fun, desired_depth, desired_depth);
		
		return std::make_optional(result_t{ control.principal_variation(), score });
	}
	catch(const detail::search_aborted_exception&)
	{
		return std::optional<result_t>{};
	}
}

template <typename SCORE_T>
class negamax_default_decision_function
{
	public:
	auto operator()(SCORE_T score) noexcept
	{
		if(score>max_score)
		{
			max_score=score;
			return search_decision::store_and_continue;
		}
		return search_decision::continue_search;
	}
	
	auto get_score() const noexcept { return max_score; }
	auto get_reversed() const noexcept { return negamax_default_decision_function{}; }
	
	static constexpr auto infinity_score() noexcept { return -(::std::numeric_limits<SCORE_T>::lowest()+1); }
	
	private:
	SCORE_T max_score=::std::numeric_limits<SCORE_T>::lowest()+1;
};

template <typename BOARD_T, typename SEARCH_CONTROL_T>
auto negamax(const BOARD_T& board, SEARCH_CONTROL_T& control, unsigned desired_depth)
{
	return negamax(board,control,negamax_default_decision_function<::std::decay_t<decltype(control.static_eval(board))>>{},[](){ return false; },desired_depth);
}

}} //end namespace philchess:algorithm

#endif
