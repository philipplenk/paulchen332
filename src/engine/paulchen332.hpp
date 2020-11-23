#ifndef PHILCHESS_ENGINE_PAULCHEN332_H
#define PHILCHESS_ENGINE_PAULCHEN332_H

#include <philchess/chessboard.hpp>
#include <philchess/default_search_control.hpp>
#include <philchess/time_manager.hpp>
#include <philchess/types.hpp>

#include <philchess/algorithm/alpha_beta_pruning.hpp>
#include <philchess/algorithm/aspiration_window_search.hpp>
#include <philchess/algorithm/iterative_deepening.hpp>
#include <philchess/algorithm/negamax.hpp>

#include <philchess/uci/types.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <optional>
#include <string_view>
#include <variant>

namespace philchess {
namespace engine
{
	using namespace std::string_view_literals;
	
	class paulchen332
	{
		public:
		constexpr static auto name="paulchen332 v0.1"sv;
		constexpr static auto authors="Philipp Lenk"sv;
		inline const static std::array<uci::option_description,1> option_list
		{{
			{"Hash"sv,uci::option_value<uci::option_type::spin>{32,0,4096}}
		}};
		
		explicit paulchen332(const philchess::search_parameters& params):
			search_control{params}
		{}
		
		paulchen332():
			paulchen332(philchess::search_parameters{})
		{}
		
		void set_option(std::integral_constant<std::size_t,0>, int hashsize_mb)
		{
			search_control.resize_tt(hashsize_mb);
		}
		
		void reset()
		{
			board.setup("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
			search_control.reset_tt();
		}
		
		void setup(std::string_view fen)
		{
			board.setup(fen);
		}
		
		void do_move(philchess::move m)
		{
			board.do_move(m);
		}
		
		template <typename SEARCH_CONTROLLER_T, typename SEARCH_SETTINGS_T>
		auto search(SEARCH_CONTROLLER_T controller, SEARCH_SETTINGS_T settings)
		{
			const unsigned max_depth=std::min(41u,settings.depth-1);
			using score_t=philchess::default_search_control::score_value_type;
			
			const auto start_time = std::chrono::high_resolution_clock::now();
			
			const auto to_move = board.side_to_move();
			std::optional<philchess::time_manager> time_mgr;
			if(settings.remaining_time[to_move])
			{
				struct time_settings_t
				{
					side_map<std::chrono::milliseconds> remaining_time{}, increment;
					unsigned moves_to_go;
				} time_settings;
				
				time_settings.remaining_time[to_move] = *settings.remaining_time[to_move]; 
				time_settings.remaining_time[reverse(to_move)] = settings.remaining_time[reverse(to_move)].value_or(std::chrono::milliseconds{0});
				time_settings.increment = settings.increment;
				time_settings.moves_to_go = settings.moves_to_go.value_or(20); //arbitrary, but seems to work quite well.
				time_mgr.emplace(time_settings, to_move);
			}
			
			const auto init_deepening = [this]()
			{
				return *philchess::algorithm::negamax(board,search_control,philchess::algorithm::alpha_beta_pruning<score_t>{},[](){ return false; }, 1); //<-- safe to dereference, as it cannot be aborted...
			};
			
			const auto search_depth = [this,controller,&time_mgr](const auto& last_result, unsigned desired_depth) mutable
			{
				controller.io.debug_message("lastEval ",last_result.eval," min ",last_result.eval-30," max ",last_result.eval+30);

				philchess::algorithm::infinity_backoff_window<philchess::algorithm::alpha_beta_pruning<score_t>,score_t> wnd{last_result.eval-30, last_result.eval+30};


				
				auto result=philchess::algorithm::aspiration_window_search(wnd,
					[this,desired_depth, controller, &time_mgr](auto decision_fun)
					{
						return philchess::algorithm::negamax(board,search_control, decision_fun,
							[controller,&time_mgr]()
							{
								return controller.should_stop || (time_mgr && time_mgr->time_is_elapsed());
							},
							desired_depth);
					},
					[controller, &time_mgr](auto failure_type, auto eval) mutable
					{
						controller.io.debug_message("eval ",eval," failed ", failure_type==philchess::algorithm::aspiration_search_result::fail_low?"low":"high");
						
						//if we reach here, our aspiration window was wrong, i.e. our assumptions were false, we are suprised by what we see. As such, extend the time we look at it to see what is going on...
						if(time_mgr && (failure_type==philchess::algorithm::aspiration_search_result::fail_low || eval<300)) //the <300 is arbitrary so, but intends to be a score that is pretty certain to be a win already
							time_mgr->try_extend();
						
						return controller.should_stop || (time_mgr && time_mgr->time_is_elapsed());
					}
				);
				return result;
			};
			
			const auto on_completed_depth = [this, controller, start_time, &time_mgr, max_depth](const auto& last_result, auto depth) mutable
			{
				const auto now=std::chrono::high_resolution_clock::now();
				const std::chrono::milliseconds elapsed=std::chrono::duration_cast<std::chrono::milliseconds>(now-start_time);
				
				controller.io.report_pv(
					{depth,search_control.max_quiescent_depth()},
					elapsed,
					search_control.number_of_statically_evaluated_nodes(),
					last_result.eval,
					search_control.mate_distance(last_result.eval),
					last_result.pv
				);

				controller.io.debug_message("number of cache hits: ",search_control.number_of_cache_hits());
				controller.io.debug_message("number of quiescent nodes: ",search_control.number_of_quiescent_nodes());
				controller.io.debug_message("number of travsered nodes: ",search_control.number_of_traversed_nodes());
				search_control.reset_stats();
				
				return controller.should_stop || (time_mgr && !time_mgr->should_attemt_new_depth()) || depth>max_depth;
			};
			
			auto result= philchess::algorithm::iterative_deepening(
				init_deepening,
				search_depth,
				on_completed_depth
			);
			
			return result.pv[0];
		}
		
		private:
		chessboard board;
		default_search_control search_control;
	};

}} //end namespace philchess:engine

#endif
