#ifndef PHILCHESS_EVAL_MOBILITY_H
#define PHILCHESS_EVAL_MOBILITY_H

#include <philchess/types.hpp>

#include <array>

namespace philchess {
namespace eval
{
	template <typename T>
	using mobility_eval_table = std::array<T,28>; //queen on open board in the center has most possible moves with 27. so all those or 0 == 28 possibilities
	
	const piece_type_map<mobility_eval_table<int>>& get_mobility_eval_tables() noexcept;
	
}} //end namespace philchess::eval

#endif
