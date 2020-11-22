#ifndef PHILCHESS_EVAL_KING_SAFETY_H
#define PHILCHESS_EVAL_KING_SAFETY_H

#include <philchess/bitboard.hpp>
#include <philchess/types.hpp>

#include <philchess/eval/material.hpp>

namespace philchess {
namespace eval
{
	inline auto get_pawnshield_masks() noexcept
	{
		struct ret_val_t
		{
			side_map<square_table<bitboard::rank>> primary, secondary;
		} ret_val;

		for(std::uint8_t id=0;id<64;++id)
		{
			square sq{id};

			if(sq.rank()<7)
			{
					ret_val.primary[side::white][sq].set(square{sq.file(),sq.rank()+1});
					if(sq.file()>0)
							ret_val.primary[side::white][sq].set(square{sq.file()-1,sq.rank()+1});
					if(sq.file()<7)
							ret_val.primary[side::white][sq].set(square{sq.file()+1,sq.rank()+1});
					
					if(sq.rank()<6)
					{
						ret_val.secondary[side::white][sq].set(square{sq.file(),sq.rank()+2});
						if(sq.file()>0)
								ret_val.secondary[side::white][sq].set(square{sq.file()-1,sq.rank()+2});
						if(sq.file()<7)
								ret_val.secondary[side::white][sq].set(square{sq.file()+1,sq.rank()+2});
					}
			}
			if(sq.rank()>0)
			{
					ret_val.primary[side::black][sq].set(square{sq.file(),sq.rank()-1});
					if(sq.file()>0)
							ret_val.primary[side::black][sq].set(square{sq.file()-1,sq.rank()-1});
					if(sq.file()<7)
							ret_val.primary[side::black][sq].set(square{sq.file()+1,sq.rank()-1});
							
					if(sq.rank()>1)
					{			
						ret_val.secondary[side::black][sq].set(square{sq.file(),sq.rank()-2});
						if(sq.file()>0)
								ret_val.secondary[side::black][sq].set(square{sq.file()-1,sq.rank()-2});
						if(sq.file()<7)
								ret_val.secondary[side::black][sq].set(square{sq.file()+1,sq.rank()-2});
					}
			}
		}

		return ret_val;
	}
	
	const material_factors<int>& get_castling_rights_table() noexcept;
	
	const material_factors<int>& get_kingsquare_attacker_table() noexcept;
	
	const material_factors<int>& get_primary_pawnshield_table() noexcept;
	
	const material_factors<int>& get_secondary_pawnshield_table() noexcept;
	
	const material_factors<int>& get_open_files_table() noexcept;
	
}} //end namespace philchess::eval

#endif
