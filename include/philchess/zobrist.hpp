#ifndef PHILCHESS_UTILITY_ZOBRIST_H
#define PHILCHESS_UTILITY_ZOBRIST_H

#include <philchess/types.hpp>

#include <ptl/constexpr_mersenne_twister.hpp>

#include <array>

#include <cstdint>

namespace philchess
{
	struct zobrist_table_t
	{
		using piecemap_t=side_map<piece_type_map<std::uint64_t>>;
		square_table<piecemap_t> board_table{};
		side_map<castling_right_map<std::uint64_t>> castling_table{};
		std::array<std::uint64_t, 9> enpassant_table{};
		std::uint64_t black_to_move{};
	};
	
	constexpr auto make_zobrist_table() noexcept
	{
		constexpr auto seed=1729;
		ptl::mersenne_twister19937_64 mt{seed};
		zobrist_table_t table;
		
		for(std::size_t i=0; i<table.board_table.size(); ++i)
		{
			auto sq=square{i};
			auto& piece_tbl=table.board_table[sq];
			for(std::uint8_t piece=0;piece<piece_tbl[side::white].size();++piece) //incredibly ugly and error prone...
			{
				piece_tbl[side::white][static_cast<piece_type>(piece)]= mt();
				piece_tbl[side::black][static_cast<piece_type>(piece)]= mt();
			}
			piece_tbl[side::white][piece_type::none]=0;
			piece_tbl[side::black][piece_type::none]=0;
			
		}
		
		table.castling_table[side::white][castling_right::none]= mt();
		table.castling_table[side::white][castling_right::kingside]= mt();
		table.castling_table[side::white][castling_right::queenside]= mt();
		table.castling_table[side::white][castling_right::both]= mt();
		table.castling_table[side::white][castling_right::both]= mt();
		table.castling_table[side::black][castling_right::none]= mt();
		table.castling_table[side::black][castling_right::kingside]= mt();
		table.castling_table[side::black][castling_right::queenside]= mt();
		table.castling_table[side::black][castling_right::both]= mt();
		
		auto enpassantp=&::std::get<0>(table.enpassant_table);
		for(std::size_t file=0;file<table.enpassant_table.size()-1;++file)
			enpassantp[file]= mt();
		enpassantp[table.enpassant_table.size()-1]=0;
			
		table.black_to_move= mt();
		
		return table;
	}
	
	class zobrist
	{
		public:
		
		constexpr void update(piece_type t, square sq, side s) noexcept { val^=table.board_table[sq][s][t]; }
		constexpr void update(castling_right r, side s) noexcept { val^=table.castling_table[s][r]; }
		constexpr void update_enpassant(std::uint8_t file) noexcept { val^=table.enpassant_table[file]; }
		constexpr void update_side() noexcept { val^=table.black_to_move; }
		
		constexpr const auto value() const noexcept { return val; }
		
		friend constexpr auto operator==(zobrist lhs, zobrist rhs) noexcept { return lhs.val==rhs.val; }
		friend constexpr auto operator!=(zobrist lhs, zobrist rhs) noexcept { return !(lhs==rhs); }
		private:
		std::uint64_t val{0};
		
		static constexpr decltype(make_zobrist_table()) table=make_zobrist_table();
	};
	
} //end namespace philchess

#endif
