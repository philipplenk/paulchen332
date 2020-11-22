#ifndef PHILCHESS_UCI_CLI_H
#define PHILCHESS_UCI_CLI_H

namespace philchess {
namespace uci
{
	template <typename ENGINE_T, typename IO_T, typename ...ARGS_T>
	void run_cli(IO_T io, ARGS_T&&... args);
	
}} //end namespace philchess:uci

#include "cli_impl.hpp"
#endif
