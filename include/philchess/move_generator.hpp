#ifndef PHILCHESS_MOVE_GENERATOR_H
#define PHILCHESS_MOVE_GENERATOR_H

#include <philchess/bitboard.hpp>
#include <philchess/types.hpp>

namespace philchess
{
	template <typename BOARD_PROXY_T>
	class move_generator
	{
		public:
		constexpr move_generator(BOARD_PROXY_T proxy) noexcept:
			proxy_{proxy}
		{}
		
		template <piece_type type, typename FUN_T> constexpr void all(square sq, side s, FUN_T add_move) const;
		
		template <typename FUN_T> constexpr void all_diagonal_bishop(square sq, side s, FUN_T add_move) const;
		template <typename FUN_T> constexpr void all_antidiagonal_bishop(square sq, side s, FUN_T add_move) const;
		template <typename FUN_T> constexpr void all_rank_rook(square sq, side s, FUN_T add_move) const;
		template <typename FUN_T> constexpr void all_file_rook(square sq, side s, FUN_T add_move) const;
		
		template <typename FUN_T> constexpr void all_king(square sq, side s, FUN_T add_move) const;
		template <typename FUN_T> constexpr void all_king_without_castle(square sq, side s, FUN_T add_move) const;
		template <typename FUN_T> constexpr void king_castle(square sq, side s, FUN_T add_move) const;
		
		template <piece_type type, typename FUN_T> constexpr void capture(square sq, side s, FUN_T add_move) const;
		
		template <typename FUN_T> constexpr void capture_diagonal_bishop(square sq, side s, FUN_T add_move) const;
		template <typename FUN_T> constexpr void capture_antidiagonal_bishop(square sq, side s, FUN_T add_move) const;
		template <typename FUN_T> constexpr void capture_rank_rook(square sq, side s, FUN_T add_move) const;
		template <typename FUN_T> constexpr void capture_file_rook(square sq, side s, FUN_T add_move) const;
		
		template <piece_type type, typename FUN_T> constexpr void non_capture(square sq, side s, FUN_T add_move) const;
		
		template <typename FUN_T> constexpr void non_capture_diagonal_bishop(square sq, side s, FUN_T add_move) const;
		template <typename FUN_T> constexpr void non_capture_antidiagonal_bishop(square sq, side s, FUN_T add_move) const;
		template <typename FUN_T> constexpr void non_capture_rank_rook(square sq, side s, FUN_T add_move) const;
		template <typename FUN_T> constexpr void non_capture_file_rook(square sq, side s, FUN_T add_move) const;
		
		template <typename FUN_T> constexpr void non_capture_king(square sq, side s, FUN_T add_move) const;
		
		template <piece_type type, typename FUN_T> constexpr void capture(square sq, side s, square target, FUN_T add_move) const;
		
		template <typename FUN_T> constexpr void capture_pawn_without_enpassant_fixed_promotion(square sq, side s, square target, FUN_T add_move) const;
		template <typename FUN_T> constexpr void capture_diagonal_bishop(square sq, side s, square target, FUN_T add_move) const;
		template <typename FUN_T> constexpr void capture_antidiagonal_bishop(square sq, side s, square target, FUN_T add_move) const;
		template <typename FUN_T> constexpr void capture_rank_rook(square sq, side s, square target, FUN_T add_move) const;
		template <typename FUN_T> constexpr void capture_file_rook(square sq, side s, square target, FUN_T add_move) const;
		
		template <piece_type type, typename FUN_T> constexpr void capture_targets(square sq, side s, bitboard::rank targets, FUN_T add_move) const;
				
		template <typename FUN_T> constexpr void non_capture_pawnset(bitboard::rank pawns, side s, FUN_T add_move) const;
		template <typename FUN_T> constexpr void capture_pawnset(bitboard::rank pawns, side s, FUN_T add_move) const;
		
		template <typename FUN_T> constexpr void capture_pawnset(bitboard::rank pawns, side s, square target, FUN_T add_move) const;
		template <typename FUN_T> constexpr void capture_pawnset_targets(bitboard::rank pawns, side s, bitboard::rank targets, FUN_T add_move) const;
		
		template <typename FUN_T> constexpr void non_capture_pawnset_targets(bitboard::rank pawns, side s, bitboard::rank targets, FUN_T add_move) const;
		
		template <typename FUN_T> constexpr void capture_pawnset_enpassant(bitboard::rank pawns, side s, FUN_T add_move) const;
		template <typename FUN_T> constexpr void capture_pawnset_enpassant(bitboard::rank pawns, side s, square target, FUN_T add_move) const;
				
		private:
		BOARD_PROXY_T proxy_;
		
		template <typename FUN_T> constexpr static void from_target_ranks(square src, bitboard::rank target_bb, FUN_T add_move);
	};
	
} //end namespace philchess

#include "move_generator_impl.hpp"
#endif
