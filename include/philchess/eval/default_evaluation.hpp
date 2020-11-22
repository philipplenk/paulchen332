#ifndef PHILCHESS_EVAL_DEFAULT_EVALUATION_H
#define PHILCHESS_EVAL_DEFAULT_EVALUATION_H

#include <philchess/bitboard.hpp>
#include <philchess/bitboard_range.hpp>
#include <philchess/bitboard_patterns.hpp>

#include <philchess/eval/king_safety.hpp>
#include <philchess/eval/material.hpp>
#include <philchess/eval/pawn_structure.hpp>

#include <ptl/bit.hpp>

#include <cstddef>

namespace philchess {
namespace eval
{
	bool is_likely_drawn(const side_map<piece_type_map<unsigned>> counts) noexcept
	{
		if
		(
			counts[side::white][piece_type::pawn]==0 && counts[side::black][piece_type::pawn]==0 &&
			counts[side::white][piece_type::queen]==0 && counts[side::black][piece_type::queen]==0
		)
		{
			const auto black_minor = counts[side::black][piece_type::bishop]+counts[side::black][piece_type::knight];
			const auto white_minor = counts[side::white][piece_type::bishop]+counts[side::white][piece_type::knight];
				
			if
			(
				(counts[side::white][piece_type::rook]==1 && counts[side::black][piece_type::rook]==0 && white_minor==0 && black_minor<3 && black_minor>0) ||
				(counts[side::black][piece_type::rook]==1 && counts[side::white][piece_type::rook]==0 && black_minor==0 && white_minor<3 && white_minor>0)
			)
				return true;
		}
		
		return false;
	}
	
	template <typename EVAL_T, typename PARAMETERS_T>
	EVAL_T default_evaluation(const chessboard& board, const PARAMETERS_T& parameters) noexcept
	{
		using ptl::popcount;
		
		EVAL_T middlegame_eval=0, endgame_eval=0, phase_independent_eval=0;
		const auto s=board.to_move_;
		const auto opponent_s=reverse(s);
		
		side_map<piece_type_map<unsigned>> material_counts{};
		piece_type_map<unsigned> total_material_counts{};
		side_map<piece_type_map<unsigned>> kingsquare_attacker_counts{};
		
		for(const auto t: {piece_type::bishop, piece_type::knight, piece_type::rook, piece_type::queen})
		{
			material_counts[s][t]=popcount(board.piece_bitboard(s,t).ranks());
			material_counts[opponent_s][t]=popcount(board.piece_bitboard(opponent_s,t).ranks());
			
			total_material_counts[t]=material_counts[s][t]+material_counts[opponent_s][t];
		}
		
		material_counts[s][piece_type::pawn]=popcount(board.piece_bitboard(s,piece_type::pawn).ranks());
		material_counts[opponent_s][piece_type::pawn]=popcount(board.piece_bitboard(opponent_s,piece_type::pawn).ranks());
		
		if(is_likely_drawn(material_counts))
			return 0;
		
		side_map<material_factor_key> material_keys{extract_and_compute_material_factor_key(material_counts[side::white]),extract_and_compute_material_factor_key(material_counts[side::black])}; //<--this might hide a bug like this, not entirely sure white is 0 and black is 1... for now i will simply overwrite it a line below, but thats only a temporary workaround while i am still developing this method...
		material_keys[s]=extract_and_compute_material_factor_key(material_counts[s]);
		material_keys[opponent_s]=extract_and_compute_material_factor_key(material_counts[opponent_s]);
				
		const auto evaluate_feature=[&, s, opponent_s](auto feature_function, auto factors)
		{
			const auto feature_s=feature_function(s);
			const auto feature_opponent_s=feature_function(opponent_s);
			
			middlegame_eval=middlegame_eval+feature_s*factors.middlegame;
			middlegame_eval=middlegame_eval-feature_opponent_s*factors.middlegame;
			endgame_eval=endgame_eval+feature_s*factors.endgame;
			endgame_eval=endgame_eval-feature_opponent_s*factors.endgame;
		};
			
		const auto evaluate_opponent_material_scaled_feature=[&, s, opponent_s](auto feature_function, const auto& factors)
		{
			const auto feature_s=feature_function(s);
			const auto feature_opponent_s=feature_function(opponent_s);
			
			phase_independent_eval=phase_independent_eval+feature_s*factors[material_keys[opponent_s]];
			phase_independent_eval=phase_independent_eval-feature_opponent_s*factors[material_keys[s]];
		};
		
		[[maybe_unused]] const auto evaluate_own_material_scaled_feature=[&, s, opponent_s](auto feature_function, const auto& factors)
		{
			const auto feature_s=feature_function(s);
			const auto feature_opponent_s=feature_function(opponent_s);
			
			phase_independent_eval=phase_independent_eval+feature_s*factors[material_keys[s]];
			phase_independent_eval=phase_independent_eval-feature_opponent_s*factors[material_keys[opponent_s]];
		};
		
		side_map<bitboard::rank> controlled{};
		
		const side_map<bitboard::rank> pawns = [&]()
		{
			side_map<bitboard::rank> ret_val;
			ret_val[side::white]=board.piece_bitboard(side::white,piece_type::pawn);
			ret_val[side::black]=board.piece_bitboard(side::black,piece_type::pawn);
			return ret_val;
		}();
		const auto pawn_info=eval::analyse_pawn_structure(pawns);
		
		const auto& occupied=board.side_occupancy_;
		
		const auto opponent_kingsquares = attacked_by_king(board.king_squares_[opponent_s]);
		const auto own_kingsquares = attacked_by_king(board.king_squares_[s]);
		
		for(auto t: {piece_type::knight, piece_type::bishop, piece_type::rook, piece_type::queen})
		{
			for(const auto p: squares(board.piece_bitboard(s,t)))
			{
				const auto attacks = attacked_by(s,t,p,board.occupancy_);
				controlled[s]|=attacks;
				kingsquare_attacker_counts[s][t]+=((attacks&opponent_kingsquares).ranks()!=0)?1:0;
				
				const auto number_of_available_moves = popcount((attacks&(~board.side_occupancy_[s])).ranks());
				phase_independent_eval=phase_independent_eval+parameters.mobility_eval_tables[t][number_of_available_moves];
			}
			
			for(const auto p: squares(board.piece_bitboard(opponent_s,t)))
			{
				const auto attacks = attacked_by(opponent_s,t,p,board.occupancy_);
				controlled[opponent_s]|=attacks;
				
				kingsquare_attacker_counts[opponent_s][t]+=((attacks&own_kingsquares).ranks()!=0)?1:0;
				
				const auto number_of_available_moves = popcount((attacks&(~board.side_occupancy_[opponent_s])).ranks());
				phase_independent_eval=phase_independent_eval-parameters.mobility_eval_tables[t][number_of_available_moves];
			}
		}
				
		controlled[s]|=own_kingsquares;
		controlled[s]|=pawn_info.attacks[s];
		
		controlled[opponent_s]|=opponent_kingsquares;	
		controlled[opponent_s]|=pawn_info.attacks[reverse(s)];
		
		for(const auto t: {piece_type::pawn, piece_type::knight, piece_type::bishop, piece_type::rook, piece_type::queen})
		{
			for(const auto p: squares(board.piece_bitboard(s,t)))
			{
				middlegame_eval=middlegame_eval+parameters.piece_square_tables[s][t][p];
				endgame_eval=endgame_eval+parameters.endgame_piece_square_tables[s][t][p];
			}
			
			for(const auto p: squares(board.piece_bitboard(opponent_s,t)))
			{
				middlegame_eval=middlegame_eval-parameters.piece_square_tables[opponent_s][t][p];
				endgame_eval=endgame_eval-parameters.endgame_piece_square_tables[opponent_s][t][p];
			}
		}
				
		middlegame_eval=middlegame_eval+parameters.piece_square_tables[s][piece_type::king][board.king_squares_[s]];
		middlegame_eval=middlegame_eval-parameters.piece_square_tables[opponent_s][piece_type::king][board.king_squares_[opponent_s]];
		
		endgame_eval=endgame_eval+parameters.endgame_piece_square_tables[s][piece_type::king][board.king_squares_[s]];
		endgame_eval=endgame_eval-parameters.endgame_piece_square_tables[opponent_s][piece_type::king][board.king_squares_[opponent_s]];
				
		side_map<material_factor_key> attacker_keys{extract_and_compute_material_factor_key(kingsquare_attacker_counts[side::white]),extract_and_compute_material_factor_key(kingsquare_attacker_counts[side::black])}; //<--this might hide a bug like this, not entirely sure white is 0 and black is 1... for now i will simply overwrite it a line below, but thats only a temporary workaround while i am still developing this method...
		attacker_keys[s]=extract_and_compute_material_factor_key(kingsquare_attacker_counts[s]);
		attacker_keys[opponent_s]=extract_and_compute_material_factor_key(kingsquare_attacker_counts[opponent_s]);
		
		[[maybe_unused]] const auto evaluate_own_attacker_scaled_feature=[&, s, opponent_s](auto feature_function, const auto& factors)
		{
			const auto feature_s=feature_function(s);
			const auto feature_opponent_s=feature_function(opponent_s);
			
			phase_independent_eval=phase_independent_eval+feature_s*factors[attacker_keys[s]];
			phase_independent_eval=phase_independent_eval-feature_opponent_s*factors[attacker_keys[opponent_s]];
		};
		
		[[maybe_unused]] const auto evaluate_opponent_attacker_scaled_feature=[&, s, opponent_s](auto feature_function, const auto& factors)
		{
			const auto feature_s=feature_function(s);
			const auto feature_opponent_s=feature_function(opponent_s);
			
			phase_independent_eval=phase_independent_eval+feature_s*factors[attacker_keys[opponent_s]];
			phase_independent_eval=phase_independent_eval-feature_opponent_s*factors[attacker_keys[s]];
		};

		const static auto pawnshield_masks=eval::get_pawnshield_masks();
		
		const auto number_of_isolated_pawns = [&](auto s) { return popcount(pawn_info.isolated[s].ranks()); };
		const auto number_of_backwards_pawns = [&](auto s) { return popcount(pawn_info.backward[s].ranks()&(~pawn_info.isolated[s].ranks())); };
		const auto number_of_doubled_pawns = [&](auto s) { return popcount(pawn_info.doubled[s].ranks()); };
		const auto number_of_passed_pawns = [&](auto s) { return popcount(pawn_info.passed[s].ranks()); };
		const auto number_of_connected_pawns = [&](auto s) { return popcount((pawn_info.attacks[s]&board.piece_bitboard(s,piece_type::pawn)).ranks()); };
		
		const auto number_of_occupied_holes = [&](auto s) { return popcount((pawn_info.holes[reverse(s)]&(board.piece_bitboard(s,piece_type::knight))).ranks()); };
		const auto number_of_rooks_on_open_files = [&](auto s) { return popcount((pawn_info.halfopen[s]&(board.piece_bitboard(s,piece_type::rook))).ranks()); };
		
		const auto number_of_controlled_squares = [&](auto s) { return popcount((controlled[s]&(~controlled[reverse(s)])).ranks()); };
		const auto number_of_hanging_pieces = [&](auto s) { return popcount((controlled[s]&(occupied[reverse(s)])).ranks()); };

		const auto number_of_attacked_kingsquares = [&](auto s) { return popcount((controlled[s]&attacked_by_king(board.king_squares_[reverse(s)])).ranks()); };
		
		const auto castling_rights_value = [&](auto s) { return parameters.castling_eval[board.castling_rights_[s]]; };
		
		const auto primary_pawnshield_value = [&](auto s) { return popcount(pawnshield_masks.primary[s][board.king_squares_[s]].ranks()&board.piece_bitboard(s,piece_type::pawn).ranks()); };
		const auto secondary_pawnshield_value = [&](auto s) { return popcount(pawnshield_masks.secondary[s][board.king_squares_[s]].ranks()&board.piece_bitboard(s,piece_type::pawn).ranks()); }; 
		const auto number_of_open_files_in_king_vicinity = [&](auto s) { return popcount(pawnshield_masks.primary[s][board.king_squares_[s]].ranks()&pawn_info.halfopen[s].ranks()); };
		
		evaluate_feature(number_of_isolated_pawns, parameters.isolated_pawn_penalty);
		evaluate_feature(number_of_backwards_pawns, parameters.backwards_pawn_penalty);
		evaluate_feature(number_of_doubled_pawns, parameters.doubled_pawn_penalty);
		evaluate_feature(number_of_passed_pawns, parameters.passed_pawn_bonus);
		evaluate_feature(number_of_connected_pawns, parameters.connected_pawn_bonus);
		
		evaluate_feature(number_of_occupied_holes, parameters.occupied_hole_bonus);
		evaluate_feature(number_of_rooks_on_open_files, parameters.rook_on_open_file_bonus);
		
		evaluate_feature(number_of_controlled_squares, parameters.controlled_square_bonus);
		evaluate_feature(number_of_hanging_pieces, parameters.hanging_piece_penalty);
		
		evaluate_own_attacker_scaled_feature(number_of_attacked_kingsquares, parameters.kingsquare_attacker_factor);
		
		evaluate_opponent_material_scaled_feature(castling_rights_value,parameters.castling_rights_factor);
		
		evaluate_opponent_material_scaled_feature(primary_pawnshield_value,parameters.primary_pawnshield_bonus);
		evaluate_opponent_material_scaled_feature(secondary_pawnshield_value,parameters.secondary_pawnshield_bonus);
		
		evaluate_opponent_material_scaled_feature(number_of_open_files_in_king_vicinity,parameters.number_of_open_files_in_king_vicinity_penalty);
		
		auto factor = (1.0*parameters.phase_factors[extract_and_compute_material_factor_key(total_material_counts)])/parameters.phase_factors[compute_material_factor_key({0,0,0,0})];
		
		return phase_independent_eval+(1.0-factor)*middlegame_eval+factor*endgame_eval;
	}
				
}} //end namespace philchess::eval

#endif
