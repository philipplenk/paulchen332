#ifndef PHILCHESS_UTILITY_BITBOARD_PATTERNS_H
#define PHILCHESS_UTILITY_BITBOARD_PATTERNS_H

#include <philchess/bitboard.hpp>
#include <philchess/types.hpp>

#include <array>

namespace philchess
{
	namespace detail
	{
		square_table<bitboard::rank> compute_knight_attacks() noexcept;
		square_table<bitboard::rank> compute_king_attacks() noexcept;
		square_table<std::array<bitboard::rank,256>> compute_rook_rank_attacks() noexcept;
		square_table<std::array<bitboard::rank,256>> compute_rook_file_attacks() noexcept;
		square_table<std::array<bitboard::rank,256>> compute_bishop_diagonal_attacks() noexcept;
		square_table<std::array<bitboard::rank,256>> compute_bishop_antidiagonal_attacks() noexcept;
	}
	
	inline bitboard::rank attacked_by_pawn(side s, square sq) noexcept
	{
		const std::uint8_t rank = (1<<sq.file());
		const auto rank_id = s==side::white?sq.rank()+1:sq.rank()-1;
		return bitboard::rank_index::from_ranks(static_cast<std::uint64_t>(static_cast<std::uint8_t>(rank<<1)|static_cast<std::uint8_t>(rank>>1))<<(8*rank_id));
	}
	
	inline bitboard::rank attacked_by_knight(square sq) noexcept
	{
		static const auto& map=detail::compute_knight_attacks();
		return map[sq];
	}
	
	inline bitboard::rank attacked_by_bishop_on_diagonal(square sq, const bitboard::all& occupancy) noexcept
	{
		static const auto& antidiagonal_map=detail::compute_bishop_antidiagonal_attacks();
		return antidiagonal_map[sq][occupancy.file_indexed_unmasked_diagonal(sq.diagonal())];
	}
	
	inline bitboard::rank attacked_by_bishop_on_antidiagonal(square sq, const bitboard::all& occupancy) noexcept
	{
		static const auto& diagonal_map=detail::compute_bishop_diagonal_attacks(); //what? why?
		return diagonal_map[sq][occupancy.file_indexed_unmasked_anti_diagonal(sq.antidiagonal())];
	}
	
	inline bitboard::rank attacked_by_bishop(square sq, const bitboard::all& occupancy) noexcept
	{
		static const auto& diagonal_map=detail::compute_bishop_diagonal_attacks();
		static const auto& antidiagonal_map=detail::compute_bishop_antidiagonal_attacks();
		return diagonal_map[sq][occupancy.file_indexed_unmasked_anti_diagonal(sq.antidiagonal())] | antidiagonal_map[sq][occupancy.file_indexed_unmasked_diagonal(sq.diagonal())];
	}
	
	inline bitboard::rank attacked_by_rook_on_file(square sq, const bitboard::all& occupancy) noexcept
	{
		static const auto& rank_map=detail::compute_rook_rank_attacks();

		return rank_map[sq][occupancy.file(sq.file())];
	}
	
	inline bitboard::rank attacked_by_rook_on_rank(square sq, const bitboard::all& occupancy) noexcept
	{
		static const auto& file_map=detail::compute_rook_file_attacks();

		return file_map[sq][occupancy.rank(sq.rank())];
	}
	
	inline bitboard::rank attacked_by_rook(square sq, const bitboard::all& occupancy) noexcept
	{
		static const auto& rank_map=detail::compute_rook_rank_attacks();
		static const auto& file_map=detail::compute_rook_file_attacks();

		return rank_map[sq][occupancy.file(sq.file())]|file_map[sq][occupancy.rank(sq.rank())];
	}
	
	inline bitboard::rank attacked_by_queen(square sq, const bitboard::all& occupancy) noexcept
	{
		return attacked_by_rook(sq,occupancy) | attacked_by_bishop(sq,occupancy);
	}
	
	inline bitboard::rank attacked_by_king(square sq) noexcept
	{
		static const auto& map=detail::compute_king_attacks();
		return map[sq];
	}
	
	inline bitboard::rank attacked_by(side s, piece_type type, square sq, const bitboard::all& occupancy) noexcept
	{
		switch(type)
		{
			case piece_type::pawn: return attacked_by_pawn(s,sq);
			case piece_type::knight: return attacked_by_knight(sq);
			case piece_type::bishop: return attacked_by_bishop(sq, occupancy);
			case piece_type::rook: return attacked_by_rook(sq, occupancy);
			case piece_type::queen: return attacked_by_queen(sq, occupancy);
			case piece_type::king: return attacked_by_king(sq);
			case piece_type::none:
			default: return {};
		}
	}
	
	template <piece_type type>
	bitboard::rank attacked_by(side s, square sq, const bitboard::all& occupancy) noexcept
	{
		if constexpr (type==piece_type::pawn)
			return attacked_by_pawn(s,sq);
		else if constexpr (type==piece_type::knight)
			return attacked_by_knight(sq);
		else if constexpr (type==piece_type::bishop)
			return attacked_by_bishop(sq,occupancy);
		else if constexpr (type==piece_type::rook)
			return attacked_by_rook(sq,occupancy);
		else if constexpr (type==piece_type::queen)
			return attacked_by_queen(sq,occupancy);
		else if constexpr (type==piece_type::king)
			return attacked_by_king(sq);
		else
			return {};
	}

} //end namespace philchess

#endif
