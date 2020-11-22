#include <philchess/chessboard.hpp>

#include <philchess/bitboard.hpp>
#include <philchess/bitboard_patterns.hpp>
#include <philchess/bitboard_range.hpp>
#include <philchess/move_generator.hpp>

#include <ptl/flatmap.hpp>

#include <cctype>

namespace
{	
	constexpr auto castling_info(philchess::move m) noexcept
	{
		struct ret_val_t
		{
			bool is_castling=false;
			philchess::square rook_origin{0}, rook_destination{0};
		} ret_val;
		
		struct castling_move_desc_t { philchess::move m; philchess::square origin, destination; };
		constexpr castling_move_desc_t candidates[] = 
		{
			{{{4,0},{6,0}}, {7,0}, {5,0}}, //white short
			{{{4,0},{2,0}}, {0,0}, {3,0}}, //white long
			{{{4,7},{6,7}}, {7,7}, {5,7}}, //black short
			{{{4,7},{2,7}}, {0,7}, {3,7}}, //black long
		};
		
		for(const auto& candidate: candidates)
		{
			if(candidate.m.from()==m.from() && candidate.m.to()==m.to())
			{
				ret_val.is_castling=true;
				ret_val.rook_origin=candidate.origin;
				ret_val.rook_destination=candidate.destination;
				break;
			}
		}
		
		return ret_val;
	}
	
} //end anonymous namespace...

using namespace philchess;

std::ostream& philchess::operator<<(std::ostream& out, const chessboard& b)
{
	for(int y=7;y>=0;--y)
	{
		out<<(y+1)<<' ';
		for(int x=0;x<8;++x)
		{
			auto type=b.piece_type_at(square{y*8+x});
			if(type==piece_type::none)
				out<<"_x_";
			else
			{
				if(b.owner_at(square{y*8+x})==side::white)
					out<<"w_";
				else
					out<<"b_";
				out<<type;
			}
			out<<' ';
		}
		out<<'\n';
	}
	
	out<<"\n_ _A_ _B_ _C_ _D_ _E_ _F_ _G_ _H_\n";
	out<<"Castling rights white: ";
	switch(b.castling_rights()[side::white])
	{
		case castling_right::both: out<<"Both"; break;
		case castling_right::kingside: out<<"Kingside"; break;
		case castling_right::queenside: out<<"Queenside"; break;
		case castling_right::none: out<<"None"; break;
	}
	out<<"\nCastling rights black: ";
	switch(b.castling_rights()[side::black])
	{
		case castling_right::both: out<<"Both"; break;
		case castling_right::kingside: out<<"Kingside"; break;
		case castling_right::queenside: out<<"Queenside"; break;
		case castling_right::none: out<<"None"; break;
	}
	out<<"\nto_move: "<<(b.side_to_move()==side::white?"white":"black")<<"\n";
	return out;
}

zobrist chessboard::calculate_zobrist_hash() const noexcept
{
	zobrist ret_val;
	for(::std::size_t i=0;i<data_.size();++i)
	{
		ret_val.update(piece_type_at(philchess::square{i}),philchess::square{i}, owner_at(philchess::square{i}));
	}
	
	if(to_move_==philchess::side::black)
		ret_val.update_side();
	
	ret_val.update(castling_rights_[philchess::side::white],philchess::side::white);
	ret_val.update(castling_rights_[philchess::side::black],philchess::side::black);

	ret_val.update_enpassant(enpassant_file_);
	
	return ret_val;
}

void chessboard::setup(std::string_view fen_string)
{
	struct board_piece {piece_type type; side s; };
	constexpr auto piece_map=ptl::make_fixed_flatmap<char, board_piece>(
	{
		{ 'r', board_piece{piece_type::rook, side::black} },
		{ 'n', board_piece{piece_type::knight, side::black} },
		{ 'b', board_piece{piece_type::bishop, side::black} },
		{ 'q', board_piece{piece_type::queen, side::black} },
		{ 'k', board_piece{piece_type::king, side::black} },
		{ 'p', board_piece{piece_type::pawn, side::black} },
		{ 'R', board_piece{piece_type::rook, side::white} },
		{ 'N', board_piece{piece_type::knight, side::white} },
		{ 'B', board_piece{piece_type::bishop, side::white} },
		{ 'Q', board_piece{piece_type::queen, side::white} },
		{ 'K', board_piece{piece_type::king, side::white} },
		{ 'P', board_piece{piece_type::pawn, side::white} },
	});
	
	std::fill(std::begin(data_),std::end(data_),piece_type::none);
	
	piece_bitboards_={};
	side_occupancy_={};
	occupancy_={};
	
	int n=0;
	
	for(int rank=7;rank>=0;--rank)
	{
		int file=0;
		char next;
		while((next=fen_string[n++])!='/' && next!=' ')
		{
			if(std::isdigit(next))
			{
				file+=next-'0';
			}
			else
			{
				auto it=piece_map.find(next);
				if(it!=piece_map.end())
				{
					square sq = square{rank*8+file};
					const auto& piece=it->second;
					data_[sq]=piece.type;
					
					occupancy_.set(sq);
					side_occupancy_[piece.s].set(sq);
					piece_bitboards_[piece.type].set(sq);
					
					if(piece.type==piece_type::king)
						king_squares_[piece.s]=square{file,rank};
	
				}
				//else error!
				++file;
			}
		}
	}
	
	auto color=fen_string[n++];
	to_move_=color=='w'?side::white:side::black;
	
	castling_rights_[side::white]=castling_rights_[side::black]=castling_right::none;
	auto castle_right=fen_string[++n];
	while(castle_right!=' ')
	{
		switch(castle_right)
		{
			case 'K': castling_rights_[side::white]=castling_rights_[side::white] | castling_right::kingside; break;
			case 'k': castling_rights_[side::black]=castling_rights_[side::black] | castling_right::kingside; break;
			case 'Q': castling_rights_[side::white]=castling_rights_[side::white] | castling_right::queenside; break;
			case 'q': castling_rights_[side::black]=castling_rights_[side::black] | castling_right::queenside; break;
			case '-': castling_rights_[side::white]=castling_rights_[side::black]=castling_right::none; break;
			default: break;
		}
		castle_right=fen_string[++n];
	}
	
	//parse en-passant
	enpassant_file_=8;
	while(fen_string[++n]==' ');
	if(auto enpassant_file_char = fen_string[n++]; enpassant_file_char!='-')
	{
		enpassant_file_=enpassant_file_char-'a';
		++n;
	}
	
	//parse halfmove clock
	fifty_move_counter_=0;
	while(fen_string[++n]!=' ')
		fifty_move_counter_=fifty_move_counter_*10+(fen_string[n]-'0');
	
	//ignore fullmove num, not needed
		
	played_moves_.clear();
	hashes_.clear();
	
	zobrist_hash_=calculate_zobrist_hash();
	
	check_inf_=compute_initial_check_info();
}

chessboard::undoable_move chessboard::do_move(philchess::move m) noexcept
{	
	const auto moved_piece=piece_type_at(m.from());
	const auto moved_piece_owner = to_move_;
	auto old_piece=piece_type_at(m.to());
	const auto old_castling=castling_rights_;
	const auto old_enpassant=enpassant_file_;
	const auto old_zobrist=zobrist_hash_;
	const auto old_movecount=fifty_move_counter_;
	const auto old_check_inf=check_inf_;
	
	const auto change_castling_rights=[&](side s, castling_right new_rights)
	{
		castling_rights_[s]=new_rights;
		
		zobrist_hash_.update(old_castling[s],s);
		zobrist_hash_.update(castling_rights_[s],s);
	};
	
	++fifty_move_counter_;
	
	if(old_piece!=piece_type::none)
		fifty_move_counter_=0;
	
	unset_piece(m.from());
	set_piece(m.to(),m.type()==move_type::promotion?m.promote_to():moved_piece,moved_piece_owner);
	
	if(moved_piece==piece_type::king)
	{
		const auto info=castling_info(m);
		if(info.is_castling)
		{
			unset_piece(info.rook_origin);
			set_piece(info.rook_destination, piece_type::rook,moved_piece_owner);
			
			m=m.as_castle();
			
			fifty_move_counter_=0;
		}
		
		change_castling_rights(to_move_,castling_right::none);
		king_squares_[moved_piece_owner]=m.to();
	}
	
	if(moved_piece==piece_type::rook)
	{
		auto rights=static_cast<int>(castling_rights_[to_move_]);
		auto to_take_away=static_cast<int>(castling_right::none);
		
		if(to_move_==side::white && m.from()==square{7,0})
			to_take_away=static_cast<int>(castling_right::kingside);
		if(to_move_==side::white && m.from()==square{0,0})
			to_take_away=static_cast<int>(castling_right::queenside);
		if(to_move_==side::black && m.from()==square{7,7})
			to_take_away=static_cast<int>(castling_right::kingside);
		if(to_move_==side::black && m.from()==square{0,7})
			to_take_away=static_cast<int>(castling_right::queenside);
			
		change_castling_rights(to_move_,static_cast<castling_right>(rights&(~to_take_away)));
	} //dreadful
	
	if(old_piece==piece_type::rook)
	{
		auto opponent_side=reverse(to_move_);
		auto rights=static_cast<int>(castling_rights_[opponent_side]);
		auto to_take_away=static_cast<int>(castling_right::none);
		
		if(opponent_side==side::white && m.to()==square{7,0})
			to_take_away=static_cast<int>(castling_right::kingside);
		if(opponent_side==side::white && m.to()==square{0,0})
			to_take_away=static_cast<int>(castling_right::queenside);
		if(opponent_side==side::black && m.to()==square{7,7})
			to_take_away=static_cast<int>(castling_right::kingside);
		if(opponent_side==side::black && m.to()==square{0,7})
			to_take_away=static_cast<int>(castling_right::queenside);
		
		change_castling_rights(opponent_side,static_cast<castling_right>(rights&(~to_take_away)));
	}
	
	zobrist_hash_.update_enpassant(enpassant_file_);
	enpassant_file_=8;
	if(moved_piece==piece_type::pawn)
	{
		fifty_move_counter_=0;
		
		auto mdist=m.to().rank()>m.from().rank()?m.to().rank()-m.from().rank():m.from().rank()-m.to().rank();
		if(mdist==2)
		{
			enpassant_file_=m.to().file();
			zobrist_hash_.update_enpassant(enpassant_file_);
		}
		
		if(m.to().file()==old_enpassant && m.to().rank()==2 && to_move_==side::black)
		{
			auto target=square{old_enpassant, 3};
			old_piece=piece_type_at(target);
			unset_piece(target);
			
			m=m.as_enpassant();
		}
		else if(m.to().file()==old_enpassant && m.to().rank()==5 && to_move_==side::white)
		{
			auto target=square{old_enpassant, 4};
			old_piece=piece_type_at(target);
			unset_piece(target);

			m=m.as_enpassant();
		}
	}
	
	to_move_=reverse(to_move_);
	zobrist_hash_.update_side();
	
	played_moves_.push_back(m);
	hashes_.push_back(old_zobrist);
	check_inf_=compute_check_info();
		
	return {m, old_piece, old_castling, old_enpassant, old_movecount, old_check_inf};
}

void chessboard::undo_move(const undoable_move m) noexcept
{
	const auto moved_piece=m.move.type()==move_type::promotion?piece_type::pawn:piece_type_at(m.move.to());
	const auto moved_piece_owner = owner_at(m.move.to());
	set_piece(m.move.to(),m.old,reverse(moved_piece_owner));
	set_piece(m.move.from(),moved_piece,moved_piece_owner);
	to_move_=reverse(to_move_);

	castling_rights_=m.castling;
	enpassant_file_=m.old_enpassant;
	
	if(moved_piece==piece_type::king)
		king_squares_[moved_piece_owner]=m.move.from();
	
	if(m.move.type()==move_type::en_passant)
	{
		unset_piece(m.move.to());
		if(m.move.to().rank()==2)
		{
			set_piece(square{enpassant_file_, 3}, m.old,reverse(moved_piece_owner));
		}
		else if(m.move.to().rank()==5)
		{
			set_piece(square{enpassant_file_, 4}, m.old,reverse(moved_piece_owner));
		}
	}
	
	if(m.move.type()==move_type::castling)
	{
		const auto info=castling_info(m.move);
		unset_piece(info.rook_destination);
		set_piece(info.rook_origin,piece_type::rook, moved_piece_owner);
	}
	
	zobrist_hash_=hashes_.back();
	
	fifty_move_counter_=m.old_movecount;
	
	check_inf_=m.check_inf;

	hashes_.pop_back();
	played_moves_.pop_back();
}

chessboard::nullmove_data chessboard::do_nullmove() noexcept
{
	const auto old_enpassant=enpassant_file_;
	const auto old_zobrist=zobrist_hash_;
	const auto old_movecount=fifty_move_counter_;
	const auto old_check_inf=check_inf_;

	fifty_move_counter_=0;

	zobrist_hash_.update_enpassant(enpassant_file_);
	enpassant_file_=8;

	to_move_=reverse(to_move_);

	zobrist_hash_.update_side();
	played_moves_.push_back(move{});
	hashes_.push_back(old_zobrist);
	check_inf_={};

	return {old_enpassant, old_movecount, old_check_inf};
}

void chessboard::undo_nullmove(nullmove_data data) noexcept
{
	to_move_=reverse(to_move_);
	enpassant_file_=data.old_enpassant;
	zobrist_hash_=hashes_.back();

	fifty_move_counter_=data.old_movecount;

	check_inf_=data.check_inf;

	hashes_.pop_back();
	played_moves_.pop_back();
}

template <typename T>
auto chessboard::checked_evasion_move_adder(T& move_container, side opponent_side) const noexcept
{
	return [&,opponent_side](philchess::move m)
	{
		if((piece_bitboard(opponent_side,piece_type::pawn)&attacked_by_pawn(reverse(opponent_side),m.to())).ranks()!=0) return;
		
		if((piece_bitboard(opponent_side,piece_type::knight)&attacked_by_knight(m.to())).ranks()!=0) return;
		if((piece_bitboard(opponent_side,piece_type::king)&attacked_by_king(m.to())).ranks()!=0) return;
		
		auto occupancy_cpy=occupancy_;
		occupancy_cpy.unset(m.from());
		const auto pseudo_bishop=attacked_by_bishop(m.to(), occupancy_cpy);
		const auto pseudo_rook=attacked_by_rook(m.to(), occupancy_cpy);
		if(((piece_bitboard(opponent_side,piece_type::queen)|piece_bitboard(opponent_side,piece_type::bishop))&pseudo_bishop).ranks()!=0) return;
		if(((piece_bitboard(opponent_side,piece_type::queen)|piece_bitboard(opponent_side,piece_type::rook))&pseudo_rook).ranks()!=0) return;

		move_container.push_back(m);
	};
}

template <typename T>
auto chessboard::checked_castle_adder(T& move_container, side opponent_side) const noexcept //assumes no check
{
	return [&, opponent_side](philchess::move m) 
	{
		const auto info=castling_info(m);
		const auto king_square=m.to();
		
		bitboard::rank target;
		target.set(king_square);
		target.set(info.rook_destination);

		const auto knight_attackers = attacked_by_knight(king_square) | attacked_by_knight(info.rook_destination);
		if((knight_attackers&piece_bitboard(opponent_side,piece_type::knight)).ranks()!=0)
			return;
		
		const auto rook_attacklines = attacked_by_rook_on_file(king_square,occupancy_) | attacked_by_rook_on_file(info.rook_destination,occupancy_);
		if((rook_attacklines&(piece_bitboard(opponent_side,piece_type::rook)|piece_bitboard(opponent_side,piece_type::queen))).ranks()!=0)
			return;
		
		const auto bishop_attacklines = attacked_by_bishop(king_square,occupancy_) | attacked_by_bishop(info.rook_destination,occupancy_);
		if((bishop_attacklines&(piece_bitboard(opponent_side,piece_type::bishop)|piece_bitboard(opponent_side,piece_type::queen))).ranks()!=0)
			return;
		
		bitboard::rank target_attackers;
		if(to_move_==side::white)
		{
			target_attackers.set(square{king_square.file(),1});
			target_attackers.set(square{king_square.file()-1,1});
			target_attackers.set(square{king_square.file()+1,1});
			target_attackers.set(square{info.rook_destination.file(),1});
			target_attackers.set(square{info.rook_destination.file()-1,1});
			target_attackers.set(square{info.rook_destination.file()+1,1});
		}
		else
		{
			target_attackers.set(square{king_square.file(),6});
			target_attackers.set(square{king_square.file()-1,6});
			target_attackers.set(square{king_square.file()+1,6});
			target_attackers.set(square{info.rook_destination.file(),6});
			target_attackers.set(square{info.rook_destination.file()-1,6});
			target_attackers.set(square{info.rook_destination.file()+1,6});
		}
		if(((piece_bitboard(opponent_side,piece_type::pawn)|piece_bitboard(opponent_side,piece_type::king))&target_attackers).ranks()!=0) return;
		
		move_container.push_back(m);
	};
}

template <typename T>
auto chessboard::checked_enpassant_move_adder(T& move_container, side opponent_side) const noexcept
{
	return [&, opponent_side](philchess::move m)
	{
		//only called for enpassant and only after checking of the moving pawn is already done
		//so all it has to do is check whether or not the enpassant pawn(the taken one) was pinned...

		auto changed_occupancy=occupancy_;
		changed_occupancy.unset(m.from());
		changed_occupancy.set(m.to());
		if(m.to().rank()==2)
			changed_occupancy.unset(square{m.to().file(),3});
		else
			changed_occupancy.unset(square{m.to().file(),4});
		
		auto king_square=king_squares_[reverse(opponent_side)];
			
		const auto bishop_pseudo=attacked_by_bishop(king_square,changed_occupancy);
		const auto rook_pseudo=attacked_by_rook_on_rank(king_square,changed_occupancy); //file attacks dont matter here, as the new pawn would block exactly the file previously blocked by the taken one...
		
		//the only thing captured in an enpassant move is a pawn, so i dont have to worry about bishop/rook attacks of the piece that was just taken...
		
		if(((piece_bitboard(opponent_side,piece_type::queen)|piece_bitboard(opponent_side,piece_type::rook))&rook_pseudo).ranks()!=0)
			return;
			
		if(((piece_bitboard(opponent_side,piece_type::queen)|piece_bitboard(opponent_side,piece_type::bishop))&bishop_pseudo).ranks()!=0)
			return;
			
		move_container.push_back(m);
	};
}

auto chessboard::generate_pinmap() const noexcept
{
	const auto opponent_side=reverse(to_move_);
	const auto king_square=king_squares_[to_move_];

	bitboard::rank attack_bishops_diagonal, attack_rooks_files;
	bitboard::rank attack_bishops_antidiagonal, attack_rooks_ranks;
	
	for(const auto& p:squares(piece_bitboard(opponent_side,piece_type::bishop) | piece_bitboard(opponent_side,piece_type::queen)))
	{
		if(p.diagonal()==king_square.diagonal())
			attack_bishops_diagonal|=attacked_by_bishop_on_diagonal(p,occupancy_);
		else if(p.antidiagonal()==king_square.antidiagonal())
			attack_bishops_antidiagonal|=attacked_by_bishop_on_antidiagonal(p,occupancy_);
	}
		
	for(const auto& p:squares(piece_bitboard(opponent_side,piece_type::rook) | piece_bitboard(opponent_side,piece_type::queen)))
	{
		if(p.file()==king_square.file())
			attack_rooks_files|=attacked_by_rook_on_file(p,occupancy_);
		else if(p.rank()==king_square.rank())
			attack_rooks_ranks|=attacked_by_rook_on_rank(p,occupancy_);
	}
	
	bitboard::rank result;
	if(attack_bishops_diagonal.ranks()!=0)
		result|=attack_bishops_diagonal&attacked_by_bishop_on_diagonal(king_square,occupancy_);
	if(attack_rooks_files.ranks()!=0)
		result|=attack_rooks_files&attacked_by_rook_on_file(king_square,occupancy_);
	if(attack_bishops_antidiagonal.ranks()!=0)
		result|=attack_bishops_antidiagonal&attacked_by_bishop_on_antidiagonal(king_square,occupancy_);
	if(attack_rooks_ranks.ranks()!=0)
		result|=attack_rooks_ranks&attacked_by_rook_on_rank(king_square,occupancy_);
				
	return result.ranks();
}

ptl::fixed_capacity_vector<philchess::move,220> chessboard::list_moves() const noexcept 
{
	move_generator<board_proxy_t> move_generator{board_proxy_t{*this}};
	
	const auto opponent_side=reverse(to_move_);
	
	ptl::fixed_capacity_vector<philchess::move,220> legal_moves;
	
	const auto king_square=king_squares_[to_move_];
	
	const auto pinned_map=generate_pinmap();
	
	const auto bitboard_iter_for_nonpinned=[&](side s, piece_type type)
	{
		return squares(bitboard::rank_index::from_ranks(piece_bitboard(s,type).ranks()&(~pinned_map)));
	};
	const auto bitboard_iter_for_nonpinned_combined=[&](side s, piece_type type0, piece_type type1)
	{
		return squares
		(
			bitboard::rank_index::from_ranks(
				(piece_bitboard(s,type0) | piece_bitboard(s,type1)).ranks()&(~pinned_map)
			)
		);
	};
	const auto bitboard_iter_for_pinned_combined=[&](side s, piece_type type0, piece_type type1)
	{
		return squares
		(
			bitboard::rank_index::from_ranks(
				(piece_bitboard(s,type0) | piece_bitboard(s,type1)).ranks()&pinned_map
			)
		);
	};

	const auto add_move=[&](auto m){ legal_moves.push_back(m); };
	const auto add_checked_evasion=checked_evasion_move_adder(legal_moves,opponent_side);
	const auto add_checked_castle=checked_castle_adder(legal_moves,opponent_side);
	const auto add_checked_enpassant=checked_enpassant_move_adder(legal_moves,opponent_side);
	
	const auto add_anti_knight_pawn_check_moves=[&]()
	{
		for(const auto& p:bitboard_iter_for_nonpinned(to_move_,piece_type::knight))
			move_generator.capture<piece_type::knight>(p,to_move_,check_inf_.checkers[0],add_move);
		
		for(const auto& p:bitboard_iter_for_nonpinned_combined(to_move_,piece_type::rook,piece_type::queen)) //can move, but its impossible to capture a checking pawn or knight whilst in pin...?
			move_generator.capture<piece_type::rook>(p,to_move_,check_inf_.checkers[0],add_move);
		
		for(const auto& p:bitboard_iter_for_nonpinned_combined(to_move_,piece_type::bishop,piece_type::queen)) //it could move along the line of its pin, but that can only be a diagonal and it cant be the diagonal a pawn is checking on (as it would not be pinned in that case...)
			move_generator.capture<piece_type::bishop>(p,to_move_,check_inf_.checkers[0],add_move);
		
		auto non_pinned_pawns = piece_bitboard(to_move_,piece_type::pawn) & bitboard::rank{bitboard::rank_index::from_ranks(~pinned_map)};
		//if its pinned it can move, but only along the path of its pin and forwards. As the king is checked, the checker cannot be on the same line/diagonal the pawn could move on, as such no moves are possible if pinned...
		move_generator.capture_pawnset(non_pinned_pawns,to_move_,check_inf_.checkers[0],add_move);
		move_generator.capture_pawnset_enpassant(non_pinned_pawns,to_move_,check_inf_.checkers[0],add_checked_enpassant);

		move_generator.all_king_without_castle(king_square,to_move_,add_checked_evasion);

	};
	
	const auto add_anti_sliding_check_moves=[&]()
	{	
		const auto attack_line=[&]()
		{
			if(check_inf_.checkers[0].file()==king_square.file() || check_inf_.checkers[0].rank()==king_square.rank())
				return attacked_by_rook(check_inf_.checkers[0],occupancy_)&attacked_by_rook(king_square,occupancy_);
			else
				return attacked_by_bishop(check_inf_.checkers[0],occupancy_)&attacked_by_bishop(king_square,occupancy_);
		}();
		
		auto attack_line_plus_checker = attack_line;
		attack_line_plus_checker.set(check_inf_.checkers[0]);
		
		const auto non_pinned_pawns = piece_bitboard(to_move_,piece_type::pawn) & bitboard::rank{bitboard::rank_index::from_ranks(~pinned_map)};
		
		move_generator.capture_pawnset(non_pinned_pawns,to_move_,check_inf_.checkers[0],add_move);
		move_generator.non_capture_pawnset_targets(non_pinned_pawns,to_move_,attack_line,add_move);
		
		for(const auto& p:bitboard_iter_for_nonpinned(to_move_,piece_type::knight))
			move_generator.capture_targets<piece_type::knight>(p,to_move_,attack_line_plus_checker,add_move);
		
		for(const auto& p:bitboard_iter_for_nonpinned_combined(to_move_,piece_type::rook,piece_type::queen))
			move_generator.capture_targets<piece_type::rook>(p,to_move_,attack_line_plus_checker,add_move);
		
		for(const auto& p:bitboard_iter_for_nonpinned_combined(to_move_,piece_type::bishop,piece_type::queen)) //could move, but not in a way that can capture checker
			move_generator.capture_targets<piece_type::bishop>(p,to_move_,attack_line_plus_checker,add_move);
		
		move_generator.all_king_without_castle(king_square,to_move_,add_checked_evasion);
	};
		
	const auto add_non_evasion_moves=[&]()
	{
		for(const auto& p:bitboard_iter_for_nonpinned(to_move_,piece_type::knight)) //cannot move if pinned(as it never ends up on same diagonl/antidiagonl/file/rank)
			move_generator.all<piece_type::knight>(p,to_move_,add_move);
		
		const auto non_pinned_pawns = piece_bitboard(to_move_,piece_type::pawn) & bitboard::rank{bitboard::rank_index::from_ranks(~pinned_map)};
		const auto pinned_pawns = piece_bitboard(to_move_,piece_type::pawn) & bitboard::rank{bitboard::rank_index::from_ranks(pinned_map)};
		
		move_generator.non_capture_pawnset(non_pinned_pawns,to_move_,add_move);
		move_generator.capture_pawnset(non_pinned_pawns,to_move_,add_move);
		move_generator.capture_pawnset_enpassant(non_pinned_pawns,to_move_,add_checked_enpassant);
		
		if(pinned_pawns.ranks()!=0)
		{
			move_generator.non_capture_pawnset(pinned_pawns,to_move_,[&](auto m)
			{
					if(m.from().file()==king_square.file())
						add_move(m);
			});
			move_generator.capture_pawnset(pinned_pawns,to_move_,[&](auto m)
			{
					if(
						(m.from().diagonal()==king_square.diagonal() && m.to().diagonal()==king_square.diagonal()) ||
						(m.from().antidiagonal()==king_square.antidiagonal() && m.to().antidiagonal()==king_square.antidiagonal())
					)
						add_move(m);
			});
			move_generator.capture_pawnset_enpassant(pinned_pawns,to_move_,[&](auto m)
			{
					if(
						(m.from().diagonal()==king_square.diagonal() && m.to().diagonal()==king_square.diagonal()) ||
						(m.from().antidiagonal()==king_square.antidiagonal() && m.to().antidiagonal()==king_square.antidiagonal())
					)
						add_checked_enpassant(m);
			});
		}
		
		for(const auto& p:bitboard_iter_for_nonpinned_combined(to_move_,piece_type::rook,piece_type::queen))
			move_generator.all<piece_type::rook>(p,to_move_,add_move);
			
		for(const auto& p:bitboard_iter_for_pinned_combined(to_move_,piece_type::rook, piece_type::queen))
		{
			if(p.rank()==king_square.rank())
				move_generator.all_rank_rook(p,to_move_,add_move);
			else if(p.file()==king_square.file())
				move_generator.all_file_rook(p,to_move_,add_move);
			/*otherwise its pinned on diagonal or antidiagonal and can do nothing about it...*/	
		}
		
		for(const auto& p:bitboard_iter_for_nonpinned_combined(to_move_,piece_type::bishop,piece_type::queen))
			move_generator.all<piece_type::bishop>(p,to_move_,add_move);
			
		for(const auto& p:bitboard_iter_for_pinned_combined(to_move_,piece_type::bishop,piece_type::queen))
		{
			if(p.diagonal()==king_square.diagonal())
				move_generator.all_diagonal_bishop(p,to_move_,add_move);
			else if(p.antidiagonal()==king_square.antidiagonal())
				move_generator.all_antidiagonal_bishop(p,to_move_,add_move);
			/*otherwise its pinned on file or rank and can do nothing about it...*/
		}

		move_generator.king_castle(king_square,to_move_,add_checked_castle);
		move_generator.all_king_without_castle(king_square,to_move_,add_checked_evasion);
	};
		
	if(check_inf_.checkers.size()==2) //only evasions possible, king must move...
	{
		move_generator.all_king_without_castle(king_square,to_move_,add_checked_evasion);
	}
	else if(check_inf_.in_check() && (piece_type_at(check_inf_.checkers[0])==piece_type::knight || piece_type_at(check_inf_.checkers[0])==piece_type::pawn))
	{
		add_anti_knight_pawn_check_moves();
	}
	else if(check_inf_.in_check())
	{
		add_anti_sliding_check_moves();
	}
	else
	{
		add_non_evasion_moves();
	}
	
	return legal_moves;
}

ptl::fixed_capacity_vector<philchess::move,220> chessboard::list_noisy_moves() const noexcept 
{
	move_generator<board_proxy_t> move_generator{board_proxy_t{*this}};
	
	const auto opponent_side=reverse(to_move_);
	
	ptl::fixed_capacity_vector<philchess::move,220> legal_moves;
	
	const auto king_square=king_squares_[to_move_];
	
	const auto pinned_map=generate_pinmap();
	
	const auto bitboard_iter_for_nonpinned=[&](side s, piece_type type)
	{
		return squares(bitboard::rank_index::from_ranks(piece_bitboard(s,type).ranks()&(~pinned_map)));
	};
	const auto bitboard_iter_for_nonpinned_combined=[&](side s, piece_type type0, piece_type type1)
	{
		return squares
		(
			bitboard::rank_index::from_ranks(
				(piece_bitboard(s,type0) | piece_bitboard(s,type1)).ranks()&(~pinned_map)
			)
		);
	};
	const auto bitboard_iter_for_pinned_combined=[&](side s, piece_type type0, piece_type type1)
	{
		return squares
		(
			bitboard::rank_index::from_ranks(
				(piece_bitboard(s,type0) | piece_bitboard(s,type1)).ranks()&pinned_map
			)
		);
	};
	
	auto non_pinned_pawns = piece_bitboard(to_move_,piece_type::pawn) & bitboard::rank{bitboard::rank_index::from_ranks(~pinned_map)};
	auto pinned_pawns = piece_bitboard(to_move_,piece_type::pawn) & bitboard::rank{bitboard::rank_index::from_ranks(pinned_map)};

	const auto add_move=[&](auto m){ legal_moves.push_back(m); };
	const auto add_checked_evasion=checked_evasion_move_adder(legal_moves,opponent_side);
	const auto add_checked_enpassant=checked_enpassant_move_adder(legal_moves,opponent_side); 
	
	if(check_inf_.in_check() && check_inf_.checkers.size()==2) //only evasions possible, king must move...
	{
		move_generator.capture<piece_type::king>(king_square,to_move_, add_checked_evasion);
	}
	else if(check_inf_.in_check())
	{ //as we only allow captures in here, we do not have to differentiate between sliding piece checks and others, as a "quiet" move to interpose a piece wont be permitted anyway...
		
		for(const auto& p: bitboard_iter_for_nonpinned(to_move_,piece_type::knight))
			move_generator.capture<piece_type::knight>(p,to_move_,check_inf_.checkers[0],add_move);
		
		for(const auto& p: bitboard_iter_for_nonpinned_combined(to_move_,piece_type::rook,piece_type::queen))
			move_generator.capture<piece_type::rook>(p,to_move_,check_inf_.checkers[0],add_move);
		
		for(const auto& p: bitboard_iter_for_nonpinned_combined(to_move_,piece_type::bishop,piece_type::queen))
			move_generator.capture<piece_type::bishop>(p,to_move_,check_inf_.checkers[0],add_move);
		
		move_generator.capture_pawnset(non_pinned_pawns,to_move_,check_inf_.checkers[0],add_move);
		move_generator.capture_pawnset_enpassant(non_pinned_pawns,to_move_,check_inf_.checkers[0],add_checked_enpassant);
		
		move_generator.capture<piece_type::king>(king_square,to_move_,add_checked_evasion);
		
	}
	else
	{	
		for(const auto& p:bitboard_iter_for_nonpinned(to_move_,piece_type::knight)) /*cannot move if pinned(as it can never land on the same rank/file/diagonal/antidiagonal after a move)*/
			move_generator.capture<piece_type::knight>(p,to_move_,add_move);
		
		for(const auto& p:bitboard_iter_for_nonpinned_combined(to_move_,piece_type::rook,piece_type::queen))
			move_generator.capture<piece_type::rook>(p,to_move_,add_move);
		
		for(const auto& p:bitboard_iter_for_pinned_combined(to_move_,piece_type::rook,piece_type::queen))
		{
			if(p.rank()==king_square.rank())
				move_generator.capture_rank_rook(p,to_move_,add_move);
			else if(p.file()==king_square.file())
				move_generator.capture_file_rook(p,to_move_,add_move);
			/*otherwise its pinned on diagonal or antidiagonal and can do nothing about it...*/					
		}
		
		for(const auto& p:bitboard_iter_for_nonpinned_combined(to_move_,piece_type::bishop,piece_type::queen))
			move_generator.capture<piece_type::bishop>(p,to_move_,add_move);
			
		for(const auto& p:bitboard_iter_for_pinned_combined(to_move_,piece_type::bishop,piece_type::queen))
		{
			if(p.diagonal()==king_square.diagonal())
				move_generator.capture_diagonal_bishop(p,to_move_,add_move);
			else if(p.antidiagonal()==king_square.antidiagonal())
				move_generator.capture_antidiagonal_bishop(p,to_move_,add_move);
			/*otherwise its pinned on file or rank and can do nothing about it...*/				
		}
		
		move_generator.capture_pawnset(non_pinned_pawns,to_move_,add_move);
		move_generator.capture_pawnset_enpassant(non_pinned_pawns,to_move_,add_checked_enpassant);
		
		//generate non-capture promotions, too
		static const auto promotion_ranks = []()
		{
			const auto bb_top=bitboard::rank{bitboard::rank_index::from_ranks(std::uint64_t{0b11111111}<<56)};
			const auto bb_bottom=bitboard::rank{bitboard::rank_index::from_ranks(std::uint64_t{0b11111111})};
			return bb_top | bb_bottom;
		}();
		
		move_generator.non_capture_pawnset_targets(non_pinned_pawns,to_move_,promotion_ranks,add_move);
		
		move_generator.capture_pawnset(pinned_pawns,to_move_,[&](auto m)
		{
			if((m.from().file()!=king_square.file() && m.from().rank()!=king_square.rank()) && 
			(
				(m.from().diagonal()==king_square.diagonal() && m.to().diagonal()==king_square.diagonal()) ||
				(m.from().antidiagonal()==king_square.antidiagonal() && m.to().antidiagonal()==king_square.antidiagonal())
			)
			)
				add_move(m);
		});
		
		move_generator.capture_pawnset_enpassant(pinned_pawns,to_move_,[&](auto m)
		{
			if((m.from().file()!=king_square.file() && m.from().rank()!=king_square.rank()) && 
			(
				(m.from().diagonal()==king_square.diagonal() && m.to().diagonal()==king_square.diagonal()) ||
				(m.from().antidiagonal()==king_square.antidiagonal() && m.to().antidiagonal()==king_square.antidiagonal())
			)
			)
				add_checked_enpassant(m);
		});
		
		move_generator.capture<piece_type::king>(king_square,to_move_,add_checked_evasion);
	}
	
	return legal_moves;
}

void chessboard::set_piece(square sq, piece_type p, side s) noexcept
{	
	const auto old_piece=piece_type_at(sq);
	const auto old_piece_owner = owner_at(sq);
	data_[sq]=p;
	
	if(p==piece_type::none)
	{
		occupancy_.unset(sq);
		side_occupancy_[old_piece_owner].unset(sq);
	}
	else
	{
		occupancy_.set(sq);
		
		side_occupancy_[s].set(sq);
		side_occupancy_[reverse(s)].unset(sq);
	}
	
	piece_bitboards_[old_piece].unset(sq);
	piece_bitboards_[p].set(sq);
	
	zobrist_hash_.update(old_piece, sq, old_piece_owner);
	zobrist_hash_.update(p, sq, s);
}

void chessboard::unset_piece(square sq) noexcept
{	
	const auto old_piece=piece_type_at(sq);
	const auto old_piece_owner = owner_at(sq);
	data_[sq]=piece_type::none;
	
	occupancy_.unset(sq);
	side_occupancy_[old_piece_owner].unset(sq);
	
	piece_bitboards_[old_piece].unset(sq);
	
	zobrist_hash_.update(old_piece, sq, old_piece_owner);
}

bool chessboard::is_in_check() const noexcept
{
	return check_info().in_check();
}

chessboard::check_info_t chessboard::check_info() const noexcept
{
	return check_inf_;
}

chessboard::check_info_t chessboard::compute_check_info() const noexcept
{
	move_generator<board_proxy_t> move_generator{board_proxy_t{*this}};
	
	check_info_t ret_val;
	const auto validate=[&](philchess::move m){ ret_val.checkers.push_back(m.from()); };
	const auto validate_reverse=[&](philchess::move m){ ret_val.checkers.push_back(m.to()); };
	
	auto king_square=king_squares_[to_move_];
	const auto opponent_side=reverse(to_move_);
	
	const auto last_move=played_moves_.back();
	
	switch(piece_type_at(last_move.to()))
	{
		case piece_type::pawn: move_generator.capture_pawn_without_enpassant_fixed_promotion(last_move.to(),opponent_side,king_square,validate); break;
		case piece_type::knight: move_generator.capture<piece_type::knight>(last_move.to(),opponent_side,king_square,validate); break;
		default: break;
	}
	
	move_generator.capture_targets<piece_type::rook>(king_square,to_move_,piece_bitboard(opponent_side,piece_type::rook)|piece_bitboard(opponent_side,piece_type::queen),validate_reverse);
	move_generator.capture_targets<piece_type::bishop>(king_square,to_move_,piece_bitboard(opponent_side,piece_type::bishop)|piece_bitboard(opponent_side,piece_type::queen),validate_reverse);
	
	return ret_val;
}

bool chessboard::would_check(philchess::move m) const noexcept
{
	move_generator<board_proxy_t> move_generator{board_proxy_t{*this}};
	
	bool check=false;
	const auto validate=[&](philchess::move m){ check=true; };
	
	const auto opponent_side=reverse(to_move_);
	auto opponent_king_square=king_squares_[opponent_side];
	
	if(m.type()==move_type::en_passant)
	{
		move_generator.capture_pawn_without_enpassant_fixed_promotion(m.to(),to_move_,opponent_king_square,validate);
		
		if(check)
			return check;
			
		auto occupancy_cpy=occupancy_;
		occupancy_cpy.unset(m.from());
		if(m.to().rank()==2)
		{
			auto target=square{enpassant_file_, 3};
			occupancy_cpy.unset(target);
		}
		else if(m.to().rank()==5)
		{
			auto target=square{enpassant_file_, 4};
			occupancy_cpy.unset(target);
		}
		
		auto rook_targets=attacked_by_rook(opponent_king_square,occupancy_cpy).ranks();
		auto bishop_targets=attacked_by_bishop(opponent_king_square,occupancy_cpy).ranks();
		
		const auto rooks_and_queens = piece_bitboard(to_move_,piece_type::rook).ranks() | piece_bitboard(to_move_,piece_type::queen).ranks();
		const auto bishops_and_queens = piece_bitboard(to_move_,piece_type::bishop).ranks() | piece_bitboard(to_move_,piece_type::queen).ranks();
		
		return (rook_targets & rooks_and_queens) || (bishop_targets & bishops_and_queens);
	}
	else if(m.type()==move_type::castling)
	{
		auto castling=castling_info(m);
		
		move_generator.capture<piece_type::rook>(castling.rook_destination,to_move_,opponent_king_square,validate);
		
		return check;
	}
	else
	{		
		switch(piece_type_at(m.from()))
		{
			case piece_type::pawn: move_generator.capture_pawn_without_enpassant_fixed_promotion(m.to(),to_move_,opponent_king_square,validate); break;
			case piece_type::knight: move_generator.capture<piece_type::knight>(m.to(),to_move_,opponent_king_square,validate); break;
			case piece_type::bishop: move_generator.capture<piece_type::bishop>(m.to(),to_move_,opponent_king_square,validate); break;
			case piece_type::rook: move_generator.capture<piece_type::rook>(m.to(),to_move_,opponent_king_square,validate); break;
			case piece_type::queen: move_generator.capture<piece_type::queen>(m.to(),to_move_,opponent_king_square,validate); break;
			case piece_type::king:
			case piece_type::none: break;
		}
		
		if(check)
			return check;
			
		if(m.type()==move_type::promotion)
		{	
			auto occupancy_cpy=occupancy_;
			occupancy_cpy.unset(m.from());
			bitboard::rank attacker;
			attacker.set(m.to());
			
			switch(m.promote_to())
			{
				case piece_type::knight: if(attacker.ranks() & attacked_by_knight(opponent_king_square).ranks()) return true; break;
				case piece_type::bishop: if(attacker.ranks() & attacked_by_bishop(opponent_king_square,occupancy_cpy).ranks()) return true; break;
				case piece_type::rook: if(attacker.ranks() & attacked_by_rook(opponent_king_square,occupancy_cpy).ranks()) return true; break;
				case piece_type::queen: if(attacker.ranks() & attacked_by_queen(opponent_king_square,occupancy_cpy).ranks()) return true; break;
				case piece_type::pawn:
				case piece_type::king:
				case piece_type::none: break;
			}
		}
		
		if(m.from().rank()==opponent_king_square.rank() && m.to().rank()!=m.from().rank())
		{	
			const auto rank_rook_attack = attacked_by_rook_on_rank(m.from(),occupancy_);
			const auto has_attacker = (rank_rook_attack.ranks() & (piece_bitboard(to_move_,piece_type::queen).ranks() | piece_bitboard(to_move_,piece_type::rook).ranks())) > 0;
			const auto attacks = (rank_rook_attack.ranks() & piece_bitboard(opponent_side,piece_type::king).ranks()) > 0;
			return has_attacker && attacks;
		}
		else if(m.from().file()==opponent_king_square.file() && m.to().file()!=m.from().file())
		{
			const auto file_rook_attack = attacked_by_rook_on_file(m.from(),occupancy_);
			const auto has_attacker = (file_rook_attack.ranks() & (piece_bitboard(to_move_,piece_type::queen).ranks() | piece_bitboard(to_move_,piece_type::rook).ranks())) > 0;
			const auto attacks = (file_rook_attack.ranks() & piece_bitboard(opponent_side,piece_type::king).ranks()) > 0;
			return has_attacker && attacks;
		}
		else if(m.from().diagonal()==opponent_king_square.diagonal() && m.to().diagonal()!=m.from().diagonal())
		{
			const auto diagonal_bishop_attack = attacked_by_bishop_on_diagonal(m.from(),occupancy_);
			const auto has_attacker = (diagonal_bishop_attack.ranks() & (piece_bitboard(to_move_,piece_type::queen).ranks() | piece_bitboard(to_move_,piece_type::bishop).ranks())) > 0;
			const auto attacks = (diagonal_bishop_attack.ranks() & piece_bitboard(opponent_side,piece_type::king).ranks()) > 0;
			return has_attacker && attacks;
		}
		else if(m.from().antidiagonal()==opponent_king_square.antidiagonal() && m.to().antidiagonal()!=m.from().antidiagonal())
		{
			const auto antidiagonal_bishop_attack = attacked_by_bishop_on_antidiagonal(m.from(),occupancy_);
			const auto has_attacker = (antidiagonal_bishop_attack.ranks() & (piece_bitboard(to_move_,piece_type::queen).ranks() | piece_bitboard(to_move_,piece_type::bishop).ranks())) > 0;
			const auto attacks = (antidiagonal_bishop_attack.ranks() & piece_bitboard(opponent_side,piece_type::king).ranks()) > 0;
			return has_attacker && attacks;
		}
		
		return false;
	}

}

chessboard::check_info_t chessboard::compute_initial_check_info() const noexcept
{
	move_generator<board_proxy_t> move_generator{board_proxy_t{*this}};
	
	check_info_t ret_val;
	
	auto king_square=king_squares_[to_move_];
	const auto opponent_side=reverse(to_move_);

	const auto validate=[&](philchess::move m){ ret_val.checkers.push_back(m.from()); };	
	
	for(const auto& p:squares(piece_bitboard(opponent_side,piece_type::pawn)))
		move_generator.capture_pawn_without_enpassant_fixed_promotion(p,opponent_side,king_square,validate);
	
	for(const auto& p:squares(piece_bitboard(opponent_side,piece_type::knight)))
		move_generator.capture<piece_type::knight>(p,opponent_side,king_square,validate);
		
	for(const auto& p:squares(piece_bitboard(opponent_side,piece_type::bishop)))
		move_generator.capture<piece_type::bishop>(p,opponent_side,king_square,validate);
		
	for(const auto& p:squares(piece_bitboard(opponent_side,piece_type::rook)))
		move_generator.capture<piece_type::rook>(p,opponent_side,king_square,validate);
		
	for(const auto& p:squares(piece_bitboard(opponent_side,piece_type::queen)))
		move_generator.capture<piece_type::queen>(p,opponent_side,king_square,validate);
	
	return ret_val;
}

bool chessboard::is_rule_draw() const noexcept
{
	if(fifty_move_counter_>=100) //<--names can be deceiving, we count plys(i.e. halfmoves), but 50 move draw happens only after 100 of those, which implies the name i chose sucks and i should change it to reflect what it really counts.
		return true;
				 
	//simplistic repetition detection
	const auto rounded_movecount=fifty_move_counter_&0xfe;
	if(rounded_movecount && hashes_.size()>1)
	{
		const auto start=hashes_.size()-rounded_movecount;
		const auto end=hashes_.size();

		for(auto idx=start; idx<end; idx+=2)
		{
			if(hashes_[idx]==zobrist_hash_)
			{
				return true;
			}
		}
	}
	
	return false;
}
