#ifndef PHILCHESS_EVAL_PIECE_SQUARE_TABLE_H
#define PHILCHESS_EVAL_PIECE_SQUARE_TABLE_H

#include <philchess/types.hpp>

namespace philchess {
namespace eval
{
	template <typename T>
	using piece_square_table=side_map<piece_type_map<square_table<T>>>;
	
	const piece_square_table<int>& get_default_piece_square_table() noexcept;
	const piece_square_table<int>& get_endgame_piece_square_table() noexcept;

}} //end namespace philchess::eval

#endif
