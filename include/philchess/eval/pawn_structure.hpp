#ifndef PHILCHESS_EVAL_PAWN_STRUCTURE_H
#define PHILCHESS_EVAL_PAWN_STRUCTURE_H

#include <philchess/bitboard.hpp>
#include <philchess/types.hpp>

namespace philchess {
namespace eval
{
	struct pawnstruct_info
	{
		side_map<bitboard::rank> isolated;
		side_map<bitboard::rank> passed;
		side_map<bitboard::rank> doubled;
		side_map<bitboard::rank> holes;
		side_map<bitboard::rank> backward;
		side_map<bitboard::rank> halfopen;
		side_map<bitboard::rank> attacks;
	};
	
	pawnstruct_info analyse_pawn_structure(const side_map<bitboard::rank>& pawns) noexcept;
}} //end namespace philchess::eval

#endif
