#include <philchess/default_search_control.hpp>

#include <philchess/eval/default_evaluation.hpp>
#include <philchess/eval/king_safety.hpp>
#include <philchess/eval/mobility.hpp>
#include <philchess/eval/phase.hpp>
#include <philchess/eval/piece_square_table.hpp>
#include <philchess/eval/see.hpp>


using namespace philchess;

int default_search_control::quiescent_search(chessboard& board, algorithm::alpha_beta_pruning<int> decision_fun, unsigned depth) noexcept
{
	++quiescent_nodes_;
	
	if(is_insufficient_material(board))
		return 0;
	
	quiescent_depth_=::std::max(depth,quiescent_depth_.load());
	
	auto cached=cached_eval(board,0);
	if(cached)
	{
		cached->eval=extend_mate_distance(cached->eval,depth);
		switch(cached->type)
		{
			case score_type::exact:
				return cached->eval;
			case score_type::lower_bound:
			{
				if(decision_fun(cached->eval)==algorithm::search_decision::cutoff)
					return decision_fun.get_score();
				break;
			}
			case score_type::upper_bound:
			{
				auto decision_fun_cpy=decision_fun; //take a copy because we should not modify the decision fun in this case
				if(decision_fun_cpy(cached->eval)==algorithm::search_decision::continue_search)
					return decision_fun_cpy.get_score();
				break;
			}
		}
	}

	const auto stand_pat=static_eval(board);
	const auto in_check=board.is_in_check();
	if(!in_check && decision_fun(stand_pat)==algorithm::search_decision::cutoff)
		return decision_fun.get_score();
	
	auto movelist=board.list_noisy_moves();
	
	if(movelist.empty() && in_check)
	{
		movelist=board.list_moves();
		if(movelist.empty())
			return mate_eval(board,depth);
		else
		{	
			for(const auto move: movelist)
			{
				auto undo_data=board.do_move(move);
					auto score=-quiescent_search(board, decision_fun.get_reversed(),depth+1);
				board.undo_move(undo_data);			
					
				switch(decision_fun(score))
				{
					case algorithm::search_decision::cutoff:
						return decision_fun.get_score();
					case algorithm::search_decision::store_and_continue: break;
					case algorithm::search_decision::continue_search: break;
				}
			}
			return decision_fun.get_score();
		}
	}
	else
	{
		if(stand_pat+parameters_.qs_delta_margin<decision_fun.get_score())
			return stand_pat;
	}
	
	const static auto& piece_square_tables=eval::get_endgame_piece_square_table();

	std::sort(std::begin(movelist),std::end(movelist),[&](auto lhs, auto rhs)
	{
		const auto lhs_aggressor=piece_square_tables[board.to_move_][board.piece_type_at(lhs.from())][lhs.from()];
		const auto rhs_aggressor=piece_square_tables[board.to_move_][board.piece_type_at(rhs.from())][rhs.from()];
		
		const auto lhs_victim=piece_square_tables[reverse(board.to_move_)][board.piece_type_at(lhs.to())][lhs.to()];
		const auto rhs_victim=piece_square_tables[reverse(board.to_move_)][board.piece_type_at(rhs.to())][rhs.to()];
		return 
			(lhs_victim>rhs_victim || (lhs_victim==rhs_victim && lhs_aggressor<rhs_aggressor));
	});
	
	for(const auto move: movelist)
	{
		if(!in_check && !philchess::eval::see_gain(board,move))
			continue;
				
		auto undo_data=board.do_move(move);
			auto score=-quiescent_search(board, decision_fun.get_reversed(),depth+1);
		board.undo_move(undo_data);
		
		switch(decision_fun(score))
		{
			case algorithm::search_decision::cutoff:
				return decision_fun.get_score();
			case algorithm::search_decision::store_and_continue: break;
			case algorithm::search_decision::continue_search: break;
		}
	}
	
	return decision_fun.get_score();
}

int default_search_control::static_eval(const chessboard& board) const noexcept
{
	++evaluated_node_num_;
		
	const static auto castling_eval_tmp=[]()
	{
		castling_right_map<int> ret_val;
		ret_val[castling_right::none]=0;
		ret_val[castling_right::kingside]=2;
		ret_val[castling_right::queenside]=1;	
		ret_val[castling_right::both]=3;
		return ret_val;
	}();
	
	struct factor_pair_t
	{
		int middlegame, endgame;
	};
		
	const static struct evaluation_parameters_t
	{
		evaluation_parameters_t():
			piece_square_tables{eval::get_default_piece_square_table()},
			endgame_piece_square_tables{eval::get_endgame_piece_square_table()},
			castling_eval{castling_eval_tmp}
		{};
		
		const std::decay_t<decltype(eval::get_default_piece_square_table())>& piece_square_tables;
		const std::decay_t<decltype(eval::get_endgame_piece_square_table())>& endgame_piece_square_tables;
		
		const std::decay_t<decltype(eval::get_mobility_eval_tables())>& mobility_eval_tables{eval::get_mobility_eval_tables()};
		
		const castling_right_map<int> castling_eval;
		
		const factor_pair_t isolated_pawn_penalty{19,-3};
		const factor_pair_t backwards_pawn_penalty{-7,-10};
		const factor_pair_t doubled_pawn_penalty{-77,-30};
		const factor_pair_t passed_pawn_bonus{99,45};
		const factor_pair_t connected_pawn_bonus{11,12};
		const factor_pair_t occupied_hole_bonus{2,18};
		const factor_pair_t rook_on_open_file_bonus{-54,-6};
		const factor_pair_t controlled_square_bonus{-8,-2};
		const factor_pair_t hanging_piece_penalty{60,22};
		
		const eval::material_factors<int>& castling_rights_factor{eval::get_castling_rights_table()};
		const eval::material_factors<int>& kingsquare_attacker_factor{eval::get_kingsquare_attacker_table()};
		const eval::material_factors<int>& primary_pawnshield_bonus{eval::get_primary_pawnshield_table()};
		const eval::material_factors<int>& secondary_pawnshield_bonus{eval::get_secondary_pawnshield_table()};
		const eval::material_factors<int>& number_of_open_files_in_king_vicinity_penalty{eval::get_open_files_table()};
		
		const eval::material_factors<int>& phase_factors{eval::get_phase_factor_table()};
		
	} parameters;
	
	return eval::default_evaluation<int>(board,parameters);
}

int default_search_control::mate_eval(const chessboard& board, unsigned depth) const noexcept
{
	++evaluated_node_num_; 
	
	if(board.is_in_check())
		return -max_mate_score+depth;
	
	return 0;
}

std::optional<int> default_search_control::mate_distance(int score) noexcept
{
	const auto abs_score=::std::abs(score);
	if(abs_score<min_mate_score || abs_score>max_mate_score)
		return std::nullopt;
	
	const auto dist=max_mate_score-abs_score;
	return score<0?-dist:dist;
}

bool default_search_control::is_insufficient_material(const chessboard& board) noexcept
{
	const bool no_non_minor_pieces = 
		board.piece_bitboard(side::white,piece_type::pawn).ranks()==0 && board.piece_bitboard(side::black,piece_type::pawn).ranks()==0 &&
		board.piece_bitboard(side::white,piece_type::rook).ranks()==0 && board.piece_bitboard(side::black,piece_type::rook).ranks()==0 &&
		board.piece_bitboard(side::white,piece_type::queen).ranks()==0 && board.piece_bitboard(side::black,piece_type::queen).ranks()==0;
	
	const auto count_b_white=ptl::popcount(board.piece_bitboard(side::white,piece_type::bishop).ranks());
	const auto count_b_black=ptl::popcount(board.piece_bitboard(side::black,piece_type::bishop).ranks());
	const auto count_k_white=ptl::popcount(board.piece_bitboard(side::white,piece_type::knight).ranks());
	const auto count_k_black=ptl::popcount(board.piece_bitboard(side::black,piece_type::knight).ranks());
	
	const auto count_minor_white=count_b_white + count_k_white;
	const auto count_minor_black=count_b_black + count_k_black;
	
	if(no_non_minor_pieces)
	{
		if((count_minor_white + count_minor_black)<2)
			return true;
	
		if(count_k_white==2 && count_b_white==0 && count_minor_black==0)
			return true;
			
		if(count_k_black==2 && count_b_black==0 && count_minor_white==0)
			return true;
	
	}
	
	return false;
}

void default_search_control::cache_eval(const chessboard& board, int eval, score_type type, move m, std::uint8_t depth) noexcept
{
	const auto zobrist_hash=board.zobrist_hash_;
	const auto idx=(zobrist_hash.value()>>(64-cache_hash_bitsize_));
	cache_[idx]={zobrist_hash,eval,m,type,depth};
}

std::optional<default_search_control::eval_t> default_search_control::cached_eval(const chessboard& board, std::uint8_t min_depth) const noexcept
{
	const auto zobrist_hash=board.zobrist_hash_;
	const auto idx=(zobrist_hash.value()>>(64-cache_hash_bitsize_));
	const auto& entry=cache_[idx];
	if(entry.depth>=min_depth && entry.zobrist_hash==zobrist_hash)
	{
		++cache_hits_;
		return std::optional<default_search_control::eval_t>{{ entry.eval, entry.type, entry.best_or_refutation_move }};
	}

	return std::nullopt;
}
