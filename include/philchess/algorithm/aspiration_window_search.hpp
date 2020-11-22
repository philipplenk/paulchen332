#ifndef PHILCHESS_ALGORITHM_ASPIRATION_WINDOW_SEARCH_H
#define PHILCHESS_ALGORITHM_ASPIRATION_WINDOW_SEARCH_H

#include <philchess/algorithm/algorithm.hpp>

#include <optional>

namespace philchess {
namespace algorithm
{
	enum class aspiration_search_result
	{
		success,
		fail_low,
		fail_high
	};
	
	template <typename WINDOW_T, typename FUN_T, typename ABORT_FUN_T>
	constexpr auto aspiration_window_search(WINDOW_T& window, FUN_T fun, ABORT_FUN_T abort)
		-> ::std::decay_t<decltype(fun(window.decision_fun()))>
	{
		for(;;)
		{
			auto current_result=fun(window.decision_fun());
			if(!current_result)
				return std::nullopt;
			
			auto window_result=window.check_and_adjust(current_result->eval);
			
			if(window_result==aspiration_search_result::success)
				return current_result;
			if(abort(window_result,current_result->eval))
				return std::nullopt;
		}
	}
	
	template <typename DECISION_FUN_T, typename SCORE_T>
	class infinity_backoff_window
	{
		public:
		constexpr infinity_backoff_window(SCORE_T init_alpha, SCORE_T init_beta) noexcept:
			alpha_{init_alpha},
			beta_{init_beta},
			fun_{alpha_,beta_}
		{}
		
		constexpr auto& decision_fun() noexcept { return fun_; }
		
		constexpr auto check_and_adjust(SCORE_T score) noexcept
		{
			if(score<=alpha_)
			{
				alpha_=-DECISION_FUN_T::infinity_score();
				fun_=DECISION_FUN_T{alpha_, beta_};
				return aspiration_search_result::fail_low;
			}
			
			if(score>=beta_)
			{
				beta_=+DECISION_FUN_T::infinity_score();
				fun_=DECISION_FUN_T{alpha_, beta_};
				return aspiration_search_result::fail_high;
			}
			
			return aspiration_search_result::success;
		}
		
		private:
		SCORE_T alpha_, beta_;
		DECISION_FUN_T fun_;
	};
	
	template <typename DECISION_FUN_T, typename SCORE_T>
	class exponential_backoff_window
	{
		public:
		constexpr exponential_backoff_window(SCORE_T init_alpha, SCORE_T init_beta) noexcept:
			alpha_{init_alpha},
			beta_{init_beta},
			step_alpha_{(init_alpha-init_beta)/2},
			step_beta_{(init_beta-init_alpha)/2},
			fun_{alpha_,beta_}
		{}
		
		constexpr auto& decision_fun() noexcept { return fun_; }
		
		constexpr auto check_and_adjust(SCORE_T score) noexcept
		{
			if(score<=alpha_)
			{
				alpha_=std::max(alpha_+step_alpha_,-DECISION_FUN_T::infinity_score());
				step_alpha_*=2;
				fun_=DECISION_FUN_T{alpha_, beta_};
				return aspiration_search_result::fail_low;
			}
			
			if(score>=beta_)
			{
				beta_=std::min(beta_+step_beta_,+DECISION_FUN_T::infinity_score());
				step_beta_*=2;
				fun_=DECISION_FUN_T{alpha_, beta_};
				return aspiration_search_result::fail_high;
			}
			
			return aspiration_search_result::success;
		}
		
		private:
		SCORE_T alpha_, beta_, step_alpha_, step_beta_;
		DECISION_FUN_T fun_;
	};
	
}} //end namespace philchess:algorithm

#endif
