#ifndef PHILCHESS_ALGORITHM_H
#define PHILCHESS_ALGORITHM_H

#include <philchess/types.hpp>

#include <optional>
#include <type_traits>

namespace philchess {
namespace algorithm
{

enum class search_decision
{
	continue_search,
	store_and_continue,
	cutoff
};

template <typename EVAL_T, typename EVAL_FUN_T, typename DECISION_FUN_T, typename STORE_FUN_T>
auto handle_cached_eval(EVAL_T cached_eval, const EVAL_FUN_T& eval, DECISION_FUN_T& decision_fun, STORE_FUN_T store_fun, unsigned depth) noexcept -> std::optional<::std::decay_t<decltype(cached_eval->eval)>>
{
	if(cached_eval)
	{
		cached_eval->eval=eval.extend_mate_distance(cached_eval->eval,depth);
			
		switch(cached_eval->type)
		{
			case score_type::exact:
			{
				if(decision_fun(cached_eval->eval)==search_decision::store_and_continue)
					store_fun(cached_eval);
				return cached_eval->eval;
			}
			case score_type::lower_bound:
			{
				switch(decision_fun(cached_eval->eval)) //we can safely continue with this as if it were a valid child eval, as its a lower bound and others can be better, but never worse and are therefore never neglegted...
				{
					case search_decision::cutoff:
						return decision_fun.get_score();
					case search_decision::store_and_continue:
						store_fun(cached_eval); break;
					default: break;
				}
				break;
			}
			case score_type::upper_bound:
			{
				auto decision_fun_cpy=decision_fun; //take a copy because we should not modify the decision fun in this case
				if(decision_fun_cpy(cached_eval->eval)==search_decision::continue_search) //assuming this is not chosen as a new best move, no other move will, as they are all worse...
					return decision_fun_cpy.get_score();
				//if the eval did not cause a cutoff we should not continue with it as a valid value, as no other one could beat it.
				break;
			}
		}
	}
	return std::nullopt;
}

}} //end namespace philchess:algorithm


#endif
