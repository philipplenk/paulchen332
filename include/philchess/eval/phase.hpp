#ifndef PHILCHESS_EVAL_PHASE_H
#define PHILCHESS_EVAL_PHASE_H

#include <philchess/eval/material.hpp>

namespace philchess {
namespace eval
{	
	const material_factors<int>& get_phase_factor_table();
	
}} //end namespace philchess::eval

#endif
