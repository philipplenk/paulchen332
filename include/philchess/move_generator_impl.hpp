#ifndef PHILCHESS_MOVEGEN_BITBOARD_BASED_MOVE_GENERATOR_IMPL_H
#define PHILCHESS_MOVEGEN_BITBOARD_BASED_MOVE_GENERATOR_IMPL_H

#include <philchess/bitboard_patterns.hpp>
#include <philchess/bitboard_range.hpp>

/*
	This should never be included directly by user code, the only
	reason it is not inside move_generator.h is for readability purposes ;-);
*/

namespace philchess
{
	
template <typename BOARD_PROXY_T>
template <typename FUN_T>
constexpr void move_generator<BOARD_PROXY_T>::from_target_ranks(square src, bitboard::rank target_bb, FUN_T add_move)
{
	for(auto target: squares(target_bb))
		add_move(move{src,target});
}

template <typename BOARD_PROXY_T>
template <piece_type type, typename FUN_T>
constexpr void move_generator<BOARD_PROXY_T>::all(square sq, side s, FUN_T add_move) const
{
	static_assert(type!=piece_type::pawn && type!=piece_type::king ,"Use more specific versions for king and pawns please ;-)");
	
	const auto targets=attacked_by<type>(s,sq,proxy_.occupancy()) & (~proxy_.occupancy(s));
	from_target_ranks(sq,targets,add_move);
}

template <typename BOARD_PROXY_T>
template <typename FUN_T>
constexpr void move_generator<BOARD_PROXY_T>::all_diagonal_bishop(square sq, side s, FUN_T add_move) const 
{
	const auto targets=attacked_by_bishop_on_diagonal(sq,proxy_.occupancy()) & (~proxy_.occupancy(s));
	from_target_ranks(sq,targets,add_move);
}

template <typename BOARD_PROXY_T>
template <typename FUN_T>
constexpr void move_generator<BOARD_PROXY_T>::all_antidiagonal_bishop(square sq, side s, FUN_T add_move) const
{
	const auto targets=attacked_by_bishop_on_antidiagonal(sq,proxy_.occupancy()) & (~proxy_.occupancy(s));
	from_target_ranks(sq,targets,add_move);
}

template <typename BOARD_PROXY_T>
template <typename FUN_T>
constexpr void move_generator<BOARD_PROXY_T>::all_rank_rook(square sq, side s, FUN_T add_move) const
{
	const auto targets= attacked_by_rook_on_rank(sq,proxy_.occupancy()) & (~proxy_.occupancy(s));
	from_target_ranks(sq,targets,add_move);
}

template <typename BOARD_PROXY_T>
template <typename FUN_T>
constexpr void move_generator<BOARD_PROXY_T>::all_file_rook(square sq, side s, FUN_T add_move) const
{
	const auto targets= attacked_by_rook_on_file(sq,proxy_.occupancy()) & (~proxy_.occupancy(s));
	from_target_ranks(sq,targets,add_move);
}

template <typename BOARD_PROXY_T>
template <typename FUN_T>
constexpr void move_generator<BOARD_PROXY_T>::all_king(square sq, side s, FUN_T add_move) const
{
	all_king_without_castle(sq,s,add_move);
	king_castle(sq,s,add_move);
}

template <typename BOARD_PROXY_T>
template <typename FUN_T>
constexpr void move_generator<BOARD_PROXY_T>::all_king_without_castle(square sq, side s, FUN_T add_move) const
{
	const auto targets= attacked_by_king(sq) & (~proxy_.occupancy(s));
	from_target_ranks(sq,targets,add_move);
}

template <typename BOARD_PROXY_T>
template <typename FUN_T>
constexpr void move_generator<BOARD_PROXY_T>::king_castle(square sq, side s, FUN_T add_move) const
{
	const auto castling=proxy_.castling_rights(s);
	if(static_cast<int>(castling)&static_cast<int>(castling_right::kingside)) //very ugly, sorry about that
	{
		const auto occ=proxy_.rank_occupancy(sq.rank());
		if((occ&0b01100000)==0)
			add_move(move{sq,square{6,sq.rank()}});
	}
	if(static_cast<int>(castling)&static_cast<int>(castling_right::queenside))
	{
		const auto occ=proxy_.rank_occupancy(sq.rank());
		if((occ&0b0001110)==0)
			add_move(move{sq,square{2,sq.rank()}});
	}
}

template <typename BOARD_PROXY_T>
template <piece_type type, typename FUN_T>
constexpr void move_generator<BOARD_PROXY_T>::capture(square sq, side s, FUN_T add_move) const
{
	static_assert(type!=piece_type::pawn, "Use more specific versions for pawns please ;-)");
	
	const auto targets=attacked_by<type>(s,sq,proxy_.occupancy()) & proxy_.occupancy(reverse(s));
	from_target_ranks(sq,targets,add_move);
}

template <typename BOARD_PROXY_T>
template <typename FUN_T>
constexpr void move_generator<BOARD_PROXY_T>::capture_diagonal_bishop(square sq, side s, FUN_T add_move) const
{
	const auto targets= attacked_by_bishop_on_diagonal(sq,proxy_.occupancy()) & proxy_.occupancy(reverse(s));
	from_target_ranks(sq,targets,add_move);
}

template <typename BOARD_PROXY_T>
template <typename FUN_T>
constexpr void move_generator<BOARD_PROXY_T>::capture_antidiagonal_bishop(square sq, side s, FUN_T add_move) const
{		
	const auto targets= attacked_by_bishop_on_antidiagonal(sq,proxy_.occupancy()) & proxy_.occupancy(reverse(s));
	from_target_ranks(sq,targets,add_move);
}

template <typename BOARD_PROXY_T>
template <typename FUN_T>
constexpr void move_generator<BOARD_PROXY_T>::capture_rank_rook(square sq, side s, FUN_T add_move) const
{
	const auto targets= attacked_by_rook_on_rank(sq,proxy_.occupancy()) & proxy_.occupancy(reverse(s));
	from_target_ranks(sq,targets,add_move);
}

template <typename BOARD_PROXY_T>
template <typename FUN_T>
constexpr void move_generator<BOARD_PROXY_T>::capture_file_rook(square sq, side s, FUN_T add_move) const
{
	const auto targets= attacked_by_rook_on_file(sq,proxy_.occupancy()) & proxy_.occupancy(reverse(s));
	from_target_ranks(sq,targets,add_move);
}

template <typename BOARD_PROXY_T>
template <piece_type type, typename FUN_T>
constexpr void move_generator<BOARD_PROXY_T>::non_capture(square sq, side s, FUN_T add_move) const
{
	static_assert(type!=piece_type::pawn && type!=piece_type::king ,"Use more specific versions for king and pawns please ;-)");
	
	const auto targets= attacked_by<type>(s,sq,proxy_.occupancy()) & (~proxy_.occupancy());
	from_target_ranks(sq,targets,add_move);
}

template <typename BOARD_PROXY_T>
template <typename FUN_T>
constexpr void move_generator<BOARD_PROXY_T>::non_capture_diagonal_bishop(square sq, side s, FUN_T add_move) const
{	
	const auto targets= attacked_by_bishop_on_diagonal(sq,proxy_.occupancy()) & (~proxy_.occupancy());
	from_target_ranks(sq,targets,add_move);
}

template <typename BOARD_PROXY_T>
template <typename FUN_T>
constexpr void move_generator<BOARD_PROXY_T>::non_capture_antidiagonal_bishop(square sq, side s, FUN_T add_move) const
{
	const auto targets= attacked_by_bishop_on_antidiagonal(sq,proxy_.occupancy()) & (~proxy_.occupancy());
	from_target_ranks(sq,targets,add_move);
}

template <typename BOARD_PROXY_T>
template <typename FUN_T>
constexpr void move_generator<BOARD_PROXY_T>::non_capture_rank_rook(square sq, side s, FUN_T add_move) const
{
	const auto targets= attacked_by_rook_on_rank(sq,proxy_.occupancy()) & (~proxy_.occupancy());
	from_target_ranks(sq,targets,add_move);
}

template <typename BOARD_PROXY_T>
template <typename FUN_T>
constexpr void move_generator<BOARD_PROXY_T>::non_capture_file_rook(square sq, side s, FUN_T add_move) const
{
	const auto targets= attacked_by_rook_on_file(sq,proxy_.occupancy()) & (~proxy_.occupancy());
	from_target_ranks(sq,targets,add_move);
}

template <typename BOARD_PROXY_T>
template <typename FUN_T>
constexpr void move_generator<BOARD_PROXY_T>::non_capture_king(square sq, side s, FUN_T add_move) const
{
	const auto targets= attacked_by_king(sq,proxy_.occupancy()) & (~proxy_.occupancy());
	from_target_ranks(sq,targets,add_move);
	
	king_castle(sq,s,add_move);
}

template <typename BOARD_PROXY_T>
template <typename FUN_T>
constexpr void move_generator<BOARD_PROXY_T>::capture_pawn_without_enpassant_fixed_promotion(square sq, side s, square to_capture, FUN_T add_move) const
{
	bitboard::rank target_bb;
	target_bb.set(to_capture);
	
	if((attacked_by_pawn(s,sq)&target_bb).ranks()!=0)
		add_move({sq,to_capture,piece_type::queen});
}

template <typename BOARD_PROXY_T>
template <piece_type type, typename FUN_T>
constexpr void move_generator<BOARD_PROXY_T>::capture(square sq, side s, square to_capture, FUN_T add_move) const
{
	static_assert(type!=piece_type::pawn, "Use more specific versions for pawns please ;-)");
	
	bitboard::rank target;
	target.set(to_capture);
	const auto targets=attacked_by<type>(s,sq,proxy_.occupancy()) & target;
	if(targets.ranks())
		add_move(move{sq,to_capture});
}

template <typename BOARD_PROXY_T>
template <typename FUN_T>
constexpr void move_generator<BOARD_PROXY_T>::capture_diagonal_bishop(square sq, side s, square to_capture, FUN_T add_move) const
{
	bitboard::rank target;
	target.set(to_capture);
	const auto targets=attacked_by_bishop_on_diagonal(s,sq,proxy_.occupancy()) & target;
	if(targets.ranks())
		add_move(move{sq,to_capture});
}

template <typename BOARD_PROXY_T>
template <typename FUN_T>
constexpr void move_generator<BOARD_PROXY_T>::capture_antidiagonal_bishop(square sq, side s, square to_capture, FUN_T add_move) const
{
	bitboard::rank target;
	target.set(to_capture);
	const auto targets=attacked_by_bishop_on_antidiagonal(s,sq,proxy_.occupancy()) & target;
	if(targets.ranks())
		add_move(move{sq,to_capture});
}

template <typename BOARD_PROXY_T>
template <typename FUN_T>
constexpr void move_generator<BOARD_PROXY_T>::capture_rank_rook(square sq, side s, square to_capture, FUN_T add_move) const
{
	bitboard::rank target;
	target.set(to_capture);
	const auto targets=attacked_by_rook_on_rank(s,sq,proxy_.occupancy()) & target;
	if(targets.ranks())
		add_move(move{sq,to_capture});
}

template <typename BOARD_PROXY_T>
template <typename FUN_T>
constexpr void move_generator<BOARD_PROXY_T>::capture_file_rook(square sq, side s, square to_capture, FUN_T add_move) const
{
	bitboard::rank target;
	target.set(to_capture);
	const auto targets=attacked_by_rook_on_file(s,sq,proxy_.occupancy()) & target;
	if(targets.ranks())
		add_move(move{sq,to_capture});
}

template <typename BOARD_PROXY_T>
template <piece_type type, typename FUN_T>
constexpr void move_generator<BOARD_PROXY_T>::capture_targets(square sq, side s, bitboard::rank targets, FUN_T add_move) const
{
	static_assert(type!=piece_type::pawn, "Use more specific versions for pawns(or rather pawnsets) please ;-)");
	
	const auto attacks = attacked_by<type>(s,sq,proxy_.occupancy());
	from_target_ranks(sq, (attacks&targets),add_move);
}

template <typename BOARD_PROXY_T>
template <typename FUN_T>
constexpr void move_generator<BOARD_PROXY_T>::non_capture_pawnset(bitboard::rank pawns, side s, FUN_T add_move) const
{
	auto pawnstops = (s==side::white?(pawns.ranks()<<8):(pawns.ranks()>>8))&~proxy_.occupancy().ranks();
	auto doublemoves = (s==side::white?((pawnstops&(std::uint64_t{0b11111111}<<16))<<8):((pawnstops&(std::uint64_t{0b11111111}<<40))>>8))&~proxy_.occupancy().ranks();
	
	while(pawnstops)
	{
		auto next=ptl::countr_zero(pawnstops);
		pawnstops^=(std::uint64_t{1}<<next);
		
		square target{next};
		square src{target.file(),target.rank()+(s==side::white?-1:1)};
		if(target.rank()==0 || target.rank()==7)
		{
			add_move(move{src,target,piece_type::queen});
			add_move(move{src,target,piece_type::rook});
			add_move(move{src,target,piece_type::knight});
			add_move(move{src,target,piece_type::bishop});
		}
		else
			add_move(move{src,target});
	}
	
	while(doublemoves)
	{
		auto next=ptl::countr_zero(doublemoves);
		doublemoves^=(std::uint64_t{1}<<next);
		
		square target{next};
		square src{target.file(),target.rank()+(s==side::white?-2:2)};
		add_move(move{src,target});
	}
}

template <typename BOARD_PROXY_T>
template <typename FUN_T>
constexpr void move_generator<BOARD_PROXY_T>::capture_pawnset(bitboard::rank pawns, side s, FUN_T add_move) const
{
	capture_pawnset_targets(pawns,s, proxy_.occupancy(reverse(s)),add_move);
}

template <typename BOARD_PROXY_T>
template <typename FUN_T>
constexpr void move_generator<BOARD_PROXY_T>::capture_pawnset(bitboard::rank pawns, side s, square target, FUN_T add_move) const
{
	bitboard::rank target_bb;
	target_bb.set(target);
	capture_pawnset_targets(pawns,s, target_bb,add_move);
}

template <typename BOARD_PROXY_T>
template <typename FUN_T>
constexpr void move_generator<BOARD_PROXY_T>::capture_pawnset_targets(bitboard::rank pawns, side s, bitboard::rank targets, FUN_T add_move) const
{
	const auto masks=[]()
	{
		struct ret_val_t
		{
			bitboard::rank all_except_left_file, all_except_right_file;
		} ret_val;
		
		ret_val.all_except_left_file=bitboard::rank{bitboard::rank::from_ranks(~std::uint64_t{0})};
		ret_val.all_except_right_file=bitboard::rank{bitboard::rank::from_ranks(~std::uint64_t{0})};
		
		for(std::size_t r=0;r<8;++r)
		{
			ret_val.all_except_left_file.unset(square{0,r});
			ret_val.all_except_right_file.unset(square{7,r});
			
		}
		return ret_val;
	}();
	
	const auto left = (pawns.ranks()<<1)& masks.all_except_left_file.ranks();
	const auto right =(pawns.ranks()>>1)& masks.all_except_right_file.ranks();
	
	auto left_attack =(s==side::white?(left<<8):(left>>8))&targets.ranks();
	auto right_attack =(s==side::white?(right<<8):(right>>8))&targets.ranks();
		
	while(left_attack)
	{
		auto next=ptl::countr_zero(left_attack);
		left_attack^=(std::uint64_t{1}<<next);
		
		square target{next};
		square src{target.file()-1,target.rank()+(s==side::white?-1:1)};
		if(target.rank()==0 || target.rank()==7)
		{
			add_move(move{src,target,piece_type::queen});
			add_move(move{src,target,piece_type::rook});
			add_move(move{src,target,piece_type::knight});
			add_move(move{src,target,piece_type::bishop});
		}
		else
			add_move(move{src,target});
	}
		
	while(right_attack)
	{
		auto next=ptl::countr_zero(right_attack);
		right_attack^=(std::uint64_t{1}<<next);
		
		square target{next};
		square src{target.file()+1,target.rank()+(s==side::white?-1:1)};
		if(target.rank()==0 || target.rank()==7)
		{
			add_move(move{src,target,piece_type::queen});
			add_move(move{src,target,piece_type::rook});
			add_move(move{src,target,piece_type::knight});
			add_move(move{src,target,piece_type::bishop});
		}
		else
			add_move(move{src,target});
	}
}

template <typename BOARD_PROXY_T>
template <typename FUN_T>
constexpr void move_generator<BOARD_PROXY_T>::non_capture_pawnset_targets(bitboard::rank pawns, side s, bitboard::rank targets, FUN_T add_move) const
{
	auto pawnstops = (s==side::white?(pawns.ranks()<<8):(pawns.ranks()>>8))&~proxy_.occupancy().ranks();
	auto doublemoves = (s==side::white?((pawnstops&(std::uint64_t{0b11111111}<<16))<<8):((pawnstops&(std::uint64_t{0b11111111}<<40))>>8))&~proxy_.occupancy().ranks();
	
	pawnstops&=targets.ranks();
	doublemoves&=targets.ranks();
	
	while(pawnstops)
	{
		auto next=ptl::countr_zero(pawnstops);
		pawnstops^=(std::uint64_t{1}<<next);
		
		square target{next};
		square src{target.file(),target.rank()+(s==side::white?-1:1)};
		if(target.rank()==0 || target.rank()==7)
		{
			add_move(move{src,target,piece_type::queen});
			add_move(move{src,target,piece_type::rook});
			add_move(move{src,target,piece_type::knight});
			add_move(move{src,target,piece_type::bishop});
		}
		else
			add_move(move{src,target});
	}
	
	while(doublemoves)
	{
		auto next=ptl::countr_zero(doublemoves);
		doublemoves^=(std::uint64_t{1}<<next);
		
		square target{next};
		square src{target.file(),target.rank()+(s==side::white?-2:2)};
		add_move(move{src,target});
	}
}

template <typename BOARD_PROXY_T>
template <typename FUN_T>
constexpr void move_generator<BOARD_PROXY_T>::capture_pawnset_enpassant(bitboard::rank pawns, side s, FUN_T add_move) const
{
	if(proxy_.enpassant_file()<8)
	{
		auto enpassant_target = square{proxy_.enpassant_file(),s==side::white?5:2};
		capture_pawnset(pawns,s,enpassant_target,[&](auto m)
		{
			add_move(m.as_enpassant());
		});
	}
}

template <typename BOARD_PROXY_T>
template <typename FUN_T>
constexpr void move_generator<BOARD_PROXY_T>::capture_pawnset_enpassant(bitboard::rank pawns, side s, square target, FUN_T add_move) const
{
	if(target.file()==proxy_.enpassant_file() && target.rank()==(s==side::white?4:3))
	{
		auto enpassant_target = square{proxy_.enpassant_file(),s==side::white?5:2};
		
		capture_pawnset(pawns,s,enpassant_target,[&](auto m)
		{
			add_move(m.as_enpassant());
		});
	}
}
	
} //end namespace philchess

#endif
