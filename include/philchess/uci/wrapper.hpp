#ifndef PHILCHESS_UCI_WRAPPER_H
#define PHILCHESS_UCI_WRAPPER_H

#include <philchess/uci/types.hpp>

#include <pcl/locked_queue.hpp>

#include <atomic>
#include <functional>
#include <thread>
#include <type_traits>

namespace philchess {
namespace uci
{	
	template <typename ENGINE_T, typename IO_T>
	class wrapper
	{
		public:
		
		template <typename... ARGS>
		wrapper(IO_T io, ARGS&& ...args);
		
		~wrapper();
		
		void uci();
		
		void debug(debug_setting setting);
		
		void isready();
		
		void setoption(option opt);
		
		void do_register(/*args?*/); //renamed due to register being reserved
		
		void ucinewgame();
		
		void position(board_position pos);
		
		void go(search_settings settings);
		
		void stop();
		
		void ponderhit();
		
		private:
		IO_T io_;
		pcl::monitor<ENGINE_T> engine_; //only accessed from within the worker thread, so the monitor is actually one more mutex than strictly necessary, but it ensures i dont do anything wrong here and I really dont trust myself with this xD...
		
		std::thread worker_thd_;
		std::atomic<bool> should_stop_;
		pcl::locked_queue<std::function<bool()>> task_queue_;
		
		std::atomic<debug_setting> debug_{debug_setting::disabled};
		
		class engine_io;
		
		template <std::size_t... idxs>
		void setoption_impl(option opt, std::index_sequence<idxs...>);
	};

}} //end namespace philchess::uci

#include "wrapper_impl.hpp"
#endif
