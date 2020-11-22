#ifndef PHILCHESS_ALGORITHM_ITERATIVE_DEEPENING_H
#define PHILCHESS_ALGORITHM_ITERATIVE_DEEPENING_H

namespace philchess {
namespace algorithm
{

template <typename INIT_FUN_T, typename ITER_FUN_T, typename ABORT_FUN_T>
constexpr auto iterative_deepening(INIT_FUN_T init_fun, ITER_FUN_T fun, ABORT_FUN_T abort_fun)
{
	unsigned depth=0;
	auto current_result=init_fun();
	
	do
	{
		auto next_result=fun(current_result,depth++);
		if(next_result)
			current_result=std::move(*next_result); 
		else
			--depth;
	}
	while(!abort_fun(current_result, depth));
	
	return current_result;
}

}} //end namespace philchess:algorithm

#endif
