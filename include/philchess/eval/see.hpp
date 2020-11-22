#ifndef PHILCHESS_EVAL_SEE_H
#define PHILCHESS_EVAL_SEE_H

#include <philchess/bitboard_patterns.hpp>
#include <philchess/chessboard.hpp>
#include <philchess/types.hpp>

#include <philchess/eval/material.hpp>

#include <ptl/bit.hpp>

#include <array>
#include <type_traits>

namespace philchess {
namespace eval
{
	template <bool want_value>
	auto see(const chessboard& board, move m, std::bool_constant<want_value> tag) noexcept
	{
		const auto& material_values=get_material_table();
		const auto first_mover=board.owner_at(m.from());
		auto to_move=first_mover;
		
		const auto side_factors=[&]()
		{
			side_map<int> factors;
			factors[to_move]=1;
			factors[reverse(to_move)]=-1;
			return factors;
		}();
		
		static const std::array attack_order{ piece_type::pawn, piece_type::knight, piece_type::bishop, piece_type::rook, piece_type::queen, piece_type::king};
		
		auto occupancy=board.occupancy_;
		auto piece_bitboards=board.piece_bitboards_;
		
		auto gain=m.type()==philchess::move_type::en_passant?material_values[philchess::piece_type::pawn]:material_values[board.piece_type_at(m.to())];
		auto price=material_values[board.piece_type_at(m.from())];
		
		occupancy.unset(m.from());
		piece_bitboards[board.piece_type_at(m.from())].unset(m.from());
		to_move=reverse(to_move);
		
		const auto take=[&](const auto& attackers, piece_type type)
		{
			auto bitboard_raw=attackers.ranks();
			std::uint64_t id=ptl::countr_zero(bitboard_raw);
			square attacking_square{id};
			
			occupancy.unset(attacking_square);
			piece_bitboards[type].unset(attacking_square);

			gain+=side_factors[to_move]*price;
			price=material_values[type];
		};
		
		const auto try_take = [&]()
		{
			std::size_t next_attacker = 0;
			while(next_attacker<attack_order.size())
			{
				const auto attacker_type=attack_order[next_attacker];
				const auto potential_attackers = piece_bitboards[attacker_type]&board.side_occupancy_[to_move];
				const auto potential_attack_squares = attacked_by(reverse(to_move),attacker_type,m.to(),occupancy);
				auto attackers = potential_attackers & potential_attack_squares;
				if(attackers.ranks()!=0)
				{
					take(attackers,attacker_type);
					return true;
				}
				++next_attacker;
			}
			return false;
		};
		
		if constexpr(want_value)
		{
			std::array<int,16> depth_gain{}; //cant ever be more than 16 pieces on the board and as such no more than that taken, obviously...
			std::size_t depth_searched=0;
			depth_gain[depth_searched]=gain;
			for(;;)
			{
				if(try_take())
				{
					depth_gain[++depth_searched]=gain;
					to_move=reverse(to_move);
				}
				else
					break;
			}
			
			to_move=reverse(to_move);
			for(;depth_searched>0;--depth_searched)
			{
				const auto f=side_factors[to_move];
				depth_gain[depth_searched-1]=f*std::max(f*depth_gain[depth_searched],f*depth_gain[depth_searched-1]);
				to_move=reverse(to_move);
			}
			
			return depth_gain[0];
		}
		else
		{
			do
			{
				if(try_take())
					to_move=reverse(to_move);
				else
					break;
			}
			while(side_factors[to_move]*gain<0 || (to_move!=first_mover && gain==0));
			
			return gain>=0;
		}
	}
	
	inline bool see_gain(const chessboard& board, move m) noexcept
	{
		return see(board,m, std::bool_constant<false>{});	
	}
	
	inline int see_value(const chessboard& board, move m) noexcept
	{
		return see(board,m, std::bool_constant<true>{});
	}
	
}} //end namespace philchess::eval

#endif
