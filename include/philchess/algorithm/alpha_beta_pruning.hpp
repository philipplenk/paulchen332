#ifndef PHILCHESS_ALGORITHM_ALPHA_BETA_PRUNING_H
#define PHILCHESS_ALGORITHM_ALPHA_BETA_PRUNING_H

#include <philchess/algorithm/algorithm.hpp>

#include <algorithm>

namespace philchess {
namespace algorithm
{

template <typename SCORE_T>
class alpha_beta_pruning
{
	public:
	constexpr alpha_beta_pruning() noexcept=default;
	constexpr alpha_beta_pruning(const alpha_beta_pruning&) noexcept=default;
	
	constexpr alpha_beta_pruning(SCORE_T alpha, SCORE_T beta) noexcept:
		alpha_and_score_{alpha},
		beta_{beta}
	{}
	
	constexpr auto operator()(SCORE_T score) noexcept
	{
		if(score>=beta_)
		{
			alpha_and_score_=beta_;
			return search_decision::cutoff;
		}
		if(score>alpha_and_score_)
		{
			alpha_and_score_=score;
			return search_decision::store_and_continue;
		}
		return search_decision::continue_search;
	}
	
	constexpr auto get_score() const noexcept { return alpha_and_score_; }
	constexpr auto get_reversed() const noexcept { auto cpy=*this; cpy.alpha_and_score_=-cpy.alpha_and_score_; cpy.beta_=-cpy.beta_; ::std::swap(cpy.alpha_and_score_,cpy.beta_); return cpy; }
	
	static constexpr auto infinity_score() { return -(::std::numeric_limits<SCORE_T>::lowest()+1); }
	
	private:
	SCORE_T alpha_and_score_=::std::numeric_limits<SCORE_T>::lowest()+1, beta_=-alpha_and_score_;
};


}} //end namespace philchess:algorithm


#endif
