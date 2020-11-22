#include <philchess/bitboard.hpp>
#include <philchess/types.hpp>

#include <philchess/eval/pawn_structure.hpp>

using namespace philchess;
using namespace philchess::eval;

namespace
{
	auto leftshift_fill(std::uint64_t base) noexcept
	{
			base|=(base<<8);
			base|=(base<<16);
			base|=(base<<32);
			
			return base;
	}
	
	auto rightshift_fill(std::uint64_t base) noexcept
	{
			base|=(base>>8);
			base|=(base>>16);
			base|=(base>>32);
			
			return base;
	}
	
	const auto masks=[]()
	{
		struct ret_val_t
		{
			bitboard::rank all_except_left_file, all_except_right_file;
		} ret_val;
		
		ret_val.all_except_left_file=bitboard::rank::from_ranks(~std::uint64_t{0});
		ret_val.all_except_right_file=bitboard::rank::from_ranks(~std::uint64_t{0});
		
		for(std::size_t r=0;r<8;++r)
		{
			ret_val.all_except_left_file.unset(square{0,r});
			ret_val.all_except_right_file.unset(square{7,r});
			
		}
		return ret_val;
	}();
	
	auto compute_neighbors(std::uint64_t base) noexcept
	{
		const auto left_shifted = ((base<<1) & masks.all_except_left_file.ranks());
		const auto right_shifted = ((base>>1) & masks.all_except_right_file.ranks());
		return left_shifted | right_shifted;
	}

	auto compute_pawn_and_attack_fills(const side_map<bitboard::rank>& pawns) noexcept
	{
		struct ret_val_t
		{
			side_map<bitboard::rank> attackfills;
			side_map<bitboard::rank> fills;
		} ret_val;
		
		const auto compute_fill=[](auto base)
		{
			return leftshift_fill(base) | rightshift_fill(base);
		};
		
		for(std::uint8_t s=0;s<2;++s)
		{
			auto current_s=static_cast<side>(s);
			
			ret_val.fills[current_s]=bitboard::rank_index::from_ranks(compute_fill(pawns[current_s].ranks()));
			
			const auto base = compute_neighbors(pawns[current_s].ranks());
			ret_val.attackfills[current_s]=bitboard::rank_index::from_ranks(compute_fill(base));
		}
		
		return ret_val;
	}
	
	auto compute_pawn_and_attack_frontspans(const side_map<bitboard::rank>& pawns) noexcept
	{	
		struct ret_val_t
		{
			side_map<bitboard::rank> frontspans;
			side_map<bitboard::rank> attackspans;
			side_map<bitboard::rank> front_and_attackspans;
			side_map<bitboard::rank> stops;
			side_map<bitboard::rank> attacks;
		} ret_val;
		
		ret_val.frontspans[side::white] = bitboard::rank_index::from_ranks(leftshift_fill(pawns[side::white].ranks()<<8));
		ret_val.frontspans[side::black] = bitboard::rank_index::from_ranks(rightshift_fill(pawns[side::black].ranks()>>8));
		
		ret_val.attacks[side::white]=bitboard::rank_index::from_ranks(compute_neighbors(pawns[side::white].ranks()<<8));
		ret_val.attacks[side::black]=bitboard::rank_index::from_ranks(compute_neighbors(pawns[side::black].ranks()>>8));

		ret_val.attackspans[side::white] = bitboard::rank_index::from_ranks(leftshift_fill(ret_val.attacks[side::white].ranks()));
		ret_val.attackspans[side::black] = bitboard::rank_index::from_ranks(rightshift_fill(ret_val.attacks[side::black].ranks()));
		
		ret_val.front_and_attackspans[side::white] = ret_val.frontspans[side::white] | ret_val.attackspans[side::white];
		ret_val.front_and_attackspans[side::black] = ret_val.frontspans[side::black] | ret_val.attackspans[side::black];
		
		ret_val.stops[side::white]=bitboard::rank_index::from_ranks(pawns[side::white].ranks()<<8);
		ret_val.stops[side::black]=bitboard::rank_index::from_ranks(pawns[side::black].ranks()>>8);
		
		return ret_val;
	}
}
	
pawnstruct_info philchess::eval::analyse_pawn_structure(const side_map<bitboard::rank>& pawns) noexcept
{
	pawnstruct_info ret_val;
	
	const auto pawn_and_attack_fills=compute_pawn_and_attack_fills(pawns);
	const auto attack_and_pawn_frontspan=compute_pawn_and_attack_frontspans(pawns);
	
	ret_val.isolated[side::white]=pawns[side::white]&(~pawn_and_attack_fills.attackfills[side::white]);
	ret_val.isolated[side::black]=pawns[side::black]&(~pawn_and_attack_fills.attackfills[side::black]);
	
	ret_val.passed[side::white]=pawns[side::white]&(~attack_and_pawn_frontspan.front_and_attackspans[side::black]);
	ret_val.passed[side::black]=pawns[side::black]&(~attack_and_pawn_frontspan.front_and_attackspans[side::white]);
	
	ret_val.doubled[side::white]=pawns[side::white]&attack_and_pawn_frontspan.frontspans[side::white];
	ret_val.doubled[side::black]=pawns[side::black]&attack_and_pawn_frontspan.frontspans[side::black];
	
	auto backward_w_shift=attack_and_pawn_frontspan.stops[side::white]&attack_and_pawn_frontspan.attacks[side::black]&(~attack_and_pawn_frontspan.attackspans[side::white]);
	auto backward_b_shift=attack_and_pawn_frontspan.stops[side::black]&attack_and_pawn_frontspan.attacks[side::white]&(~attack_and_pawn_frontspan.attackspans[side::black]);
	
	ret_val.backward[side::white]=bitboard::rank_index::from_ranks(backward_w_shift.ranks()>>8);
	ret_val.backward[side::black]=bitboard::rank_index::from_ranks(backward_b_shift.ranks()<<8);
	
	static const auto sidemasks=[]()
	{
		side_map<bitboard::rank> masks;
		
		for(std::uint_fast8_t f=0;f<8;++f)
		{
			for(std::uint_fast8_t r=0;r<4;++r)
				masks[side::white].set(square{f,r});
			for(std::uint_fast8_t r=4;r<8;++r)
				masks[side::black].set(square{f,r});
		}
		
		return masks;
	}();
	
	ret_val.holes[side::white]=attack_and_pawn_frontspan.attacks[side::black]&(~attack_and_pawn_frontspan.attackspans[side::white])&sidemasks[side::white];
	ret_val.holes[side::black]=attack_and_pawn_frontspan.attacks[side::white]&(~attack_and_pawn_frontspan.attackspans[side::black])&sidemasks[side::black];
	
	ret_val.halfopen[side::white]=~pawn_and_attack_fills.fills[side::white];
	ret_val.halfopen[side::black]=~pawn_and_attack_fills.fills[side::black];
	
	ret_val.attacks[side::white]=attack_and_pawn_frontspan.attacks[side::white];
	ret_val.attacks[side::black]=attack_and_pawn_frontspan.attacks[side::black];
		
	return ret_val;
}
