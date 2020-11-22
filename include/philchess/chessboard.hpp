#ifndef PHILCHESS_CHESSBOARD_H
#define PHILCHESS_CHESSBOARD_H

#include <philchess/bitboard.hpp>
#include <philchess/types.hpp>
#include <philchess/zobrist.hpp>

#include <philchess/algorithm/alpha_beta_pruning.hpp>
#include <philchess/algorithm/negamax.hpp>

#include <philchess/eval/piece_square_table.hpp>

#include <ptl/fixed_capacity_vector.hpp>

#include <algorithm>
#include <array>
#include <string_view>
#include <vector>

#include <cstdint>

namespace philchess
{
	//needed for the friend declaration below...
	class chessboard;
	namespace eval
	{
		template <typename EVAL_T, typename PARAMETERS_T>
		EVAL_T default_evaluation(const chessboard& board, const PARAMETERS_T& parameters) noexcept;
		
		template <bool want_value>
		auto see(const chessboard& board, move m, std::bool_constant<want_value> tag) noexcept;
	}
	
	class chessboard
	{
		public:
		chessboard() noexcept
		{
			played_moves_.reserve(256);
			hashes_.reserve(256);
		}
		
		struct check_info_t
		{
			bool in_check() const noexcept { return !checkers.empty(); }
			ptl::fixed_capacity_vector<philchess::square,2> checkers;
		};
		
		void setup(std::string_view fen_string);
		
		struct undoable_move { philchess::move move; piece_type old; side_map<castling_right> castling; std::uint8_t old_enpassant; std::uint8_t old_movecount; check_info_t check_inf; };
		
		undoable_move do_move(philchess::move m) noexcept;
		void undo_move(undoable_move m) noexcept;
		
		struct nullmove_data{ std::uint8_t old_enpassant; std::uint8_t old_movecount; check_info_t check_inf; }; 
		
		nullmove_data do_nullmove() noexcept;
		void undo_nullmove(nullmove_data data) noexcept;
		
		bool last_move_was_null() const noexcept { return !played_moves_.empty() && played_moves_.back().to()==played_moves_.back().from(); }
		
		ptl::fixed_capacity_vector<philchess::move,220> list_moves() const noexcept; //(220 seems to be an agreed upon upper bound on number of moves in any given position...)
		ptl::fixed_capacity_vector<philchess::move,220> list_noisy_moves() const noexcept;
		
		side side_to_move() const noexcept { return to_move_; }
		
		bool would_check(philchess::move m) const noexcept;
		bool is_in_check() const noexcept;
		bool is_rule_draw() const noexcept;
		
		auto hash() const noexcept { return zobrist_hash_; }
		
		constexpr auto owner_at(square sq) const noexcept
		{
			return side_occupancy_[side::white].ranks()&(std::uint64_t{1}<<sq.id())?
				side::white:
				side::black;
		}
		
		constexpr auto piece_type_at(square sq) const noexcept
		{
			return data_[sq];
		}
		
		constexpr auto castling_rights() const noexcept
		{
			return castling_rights_;
		}
		
		friend class default_search_control;
			
		template <typename EVAL_T, typename PARAMETERS_T>
		friend EVAL_T philchess::eval::default_evaluation(const chessboard& board, const PARAMETERS_T& parameters) noexcept;
		
		template <bool want_value>
		friend auto philchess::eval::see(const chessboard& board, move m, std::bool_constant<want_value> tag) noexcept;
		
		private:
		square_table<piece_type> data_;
		side to_move_=side::white;
		
		side_map<castling_right> castling_rights_{{{ castling_right::both, castling_right::both }}};
		std::uint8_t enpassant_file_=8; //invalid, somehow should make this more clear and get rid of the stupid number
		
		zobrist zobrist_hash_;
				
		bitboard::all occupancy_;
		side_map<bitboard::rank> side_occupancy_;
		
		piece_type_map<bitboard::rank> piece_bitboards_;
		
		side_map<square> king_squares_;
		
		std::vector<move> played_moves_;
		std::vector<zobrist> hashes_;
		
		std::uint8_t fifty_move_counter_=0;
		
		friend struct board_proxy_t;
		struct board_proxy_t
		{
			const chessboard& board;
			
			constexpr auto enpassant_file() const noexcept { return board.enpassant_file_; }
			constexpr auto rank_occupancy(::std::uint8_t rank_id) const noexcept { return board.occupancy_.rank(rank_id); }
			constexpr auto occupancy() const noexcept { return board.occupancy_; }
			constexpr auto occupancy(side s) const noexcept { return board.side_occupancy_[s]; }
			constexpr auto castling_rights(side s) const noexcept { return board.castling_rights_[s]; }
		};
		
		void set_piece(square sq, piece_type p, side s) noexcept;
		void unset_piece(square sq) noexcept;
		
		check_info_t check_info() const noexcept;
		check_info_t compute_check_info() const noexcept;
		check_info_t compute_initial_check_info() const noexcept;
		check_info_t check_inf_;
		
		template <typename T>
		auto checked_evasion_move_adder(T& move_container, side opponent_side) const noexcept;
		
		template <typename T>
		auto checked_castle_adder(T& move_container, side opponent_side) const noexcept;
		
		template <typename T>
		auto checked_enpassant_move_adder(T& move_container, side opponent_side) const noexcept;
		
		auto generate_pinmap() const noexcept;
		
		zobrist calculate_zobrist_hash() const noexcept;
		
		constexpr auto piece_bitboard(side s, piece_type type) const noexcept
		{
			return piece_bitboards_[type]&side_occupancy_[s];
		}
	};
	
	std::ostream& operator<<(std::ostream& out, const chessboard& b);
	
} //end namespace philchess:engine

#endif
