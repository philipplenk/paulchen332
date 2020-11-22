#ifndef PHILCHESS_TIME_MANAGER_H
#define PHILCHESS_TIME_MANAGER_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace philchess
{
	class time_manager
	{
		public:
		template <typename settings_type>
		explicit time_manager(const settings_type& settings, philchess::side to_move) noexcept:
			start_time_{std::chrono::high_resolution_clock::now()},
			usable_time_{compute_maximum_usable_time(settings.remaining_time[to_move],settings.increment[to_move],settings.moves_to_go)},
			max_thinking_time_{allocate_initial_time(settings.remaining_time[to_move],settings.increment[to_move],settings.moves_to_go)},
			min_thinking_time_{7*max_thinking_time_/10},
			remaining_sleep_time_{max_thinking_time_},
			waiting_thread_{[this]()
			{
				std::unique_lock<std::mutex> lock{m_};
				while(!destroyed_ && remaining_sleep_time_>::std::chrono::milliseconds{0})
				{
					auto timeout=remaining_sleep_time_;
					remaining_sleep_time_=::std::chrono::milliseconds{0};
					cv_.wait_for(lock,timeout,[this](){ return destroyed_; });
				}
				timeout_triggered_=true;
			}}
			
		{}
		
		~time_manager() noexcept
		{
			destroyed_=true;
			cv_.notify_all();
			waiting_thread_.join();
		}
		
		bool time_is_elapsed() const noexcept
		{
			return timeout_triggered_;
		}
		
		bool should_attemt_new_depth() const noexcept
		{
			auto now=std::chrono::high_resolution_clock::now();
			std::chrono::milliseconds elapsed=std::chrono::duration_cast<std::chrono::milliseconds>(now-start_time_);
			
			return elapsed<min_thinking_time_;
		}
		
		void try_extend() noexcept
		{		
			const auto now=std::chrono::high_resolution_clock::now();
			const auto elapsed=std::chrono::duration_cast<std::chrono::milliseconds>(now-start_time_);
			
			const auto remaining_useable_time = usable_time_-max_thinking_time_;
			
			if(remaining_useable_time>elapsed*2)
			{
				min_thinking_time_+=elapsed;
				max_thinking_time_+=elapsed*2;
				extend_waiting_thread(elapsed*2);
			}
		}
		
		private:
		const std::chrono::high_resolution_clock::time_point start_time_;
		const std::chrono::milliseconds usable_time_;
	
		std::chrono::milliseconds max_thinking_time_, min_thinking_time_;
		
		std::condition_variable cv_;
		std::mutex m_;
		std::atomic<bool> timeout_triggered_ = false;
		std::chrono::milliseconds remaining_sleep_time_;
		
		bool destroyed_ = false;
		
		std::thread waiting_thread_;
		
		void extend_waiting_thread(std::chrono::milliseconds extension) noexcept
		{
			std::unique_lock<std::mutex> lock{m_};
			remaining_sleep_time_+=extension;	
		}
		
		static std::chrono::milliseconds compute_maximum_usable_time(std::chrono::milliseconds remaining, std::chrono::milliseconds increment, unsigned moves_to_go) noexcept
		{
			const auto per_move_safety=std::max(std::chrono::milliseconds{0},std::chrono::milliseconds{20}-increment);
			const auto safety_margin=moves_to_go*per_move_safety+std::chrono::milliseconds{40}; //arbitrary, chosen by simple experiment. 

			return remaining>safety_margin?
				remaining-safety_margin:
				remaining/2; //alright, if it comes to this, we already have less time available than our safety margin, we should not stretch it. Can't really use 0 time, as that would not yield any move, but really shouldn't use all of it either, so half might be a decent guess...
		}
		
		std::chrono::milliseconds allocate_initial_time(std::chrono::milliseconds remaining, std::chrono::milliseconds increment, unsigned moves_to_go) const noexcept
		{
			const auto remaining_time=remaining+increment*moves_to_go;			
			const auto time_per_move=moves_to_go==0?remaining_time:remaining_time/moves_to_go;
			
			return std::min(time_per_move,usable_time_);
		}
		
	};
} //end namespace philchess:engine

#endif
