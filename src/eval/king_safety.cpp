#include <philchess/eval/king_safety.hpp>

using namespace philchess;

namespace
{			
	constexpr auto init_castling_rights_table() noexcept
	{
		eval::material_factors<int> ret_val{};
		ret_val[eval::compute_material_factor_key({0,0,0,0})]=19;
		ret_val[eval::compute_material_factor_key({0,0,0,1})]=31;
		ret_val[eval::compute_material_factor_key({0,0,1,0})]=-8;
		ret_val[eval::compute_material_factor_key({0,0,1,1})]=-61;
		ret_val[eval::compute_material_factor_key({0,0,2,0})]=3;
		ret_val[eval::compute_material_factor_key({0,0,2,1})]=14;
		ret_val[eval::compute_material_factor_key({0,1,0,0})]=17;
		ret_val[eval::compute_material_factor_key({0,1,0,1})]=42;
		ret_val[eval::compute_material_factor_key({0,1,1,0})]=16;
		ret_val[eval::compute_material_factor_key({0,1,1,1})]=7;
		ret_val[eval::compute_material_factor_key({0,1,2,0})]=22;
		ret_val[eval::compute_material_factor_key({0,1,2,1})]=20;
		ret_val[eval::compute_material_factor_key({0,2,0,0})]=17;
		ret_val[eval::compute_material_factor_key({0,2,0,1})]=7;
		ret_val[eval::compute_material_factor_key({0,2,1,0})]=1;
		ret_val[eval::compute_material_factor_key({0,2,1,1})]=38;
		ret_val[eval::compute_material_factor_key({0,2,2,0})]=-1;
		ret_val[eval::compute_material_factor_key({0,2,2,1})]=30;
		ret_val[eval::compute_material_factor_key({1,0,0,0})]=17;
		ret_val[eval::compute_material_factor_key({1,0,0,1})]=23;
		ret_val[eval::compute_material_factor_key({1,0,1,0})]=-9;
		ret_val[eval::compute_material_factor_key({1,0,1,1})]=-17;
		ret_val[eval::compute_material_factor_key({1,0,2,0})]=8;
		ret_val[eval::compute_material_factor_key({1,0,2,1})]=29;
		ret_val[eval::compute_material_factor_key({1,1,0,0})]=43;
		ret_val[eval::compute_material_factor_key({1,1,0,1})]=49;
		ret_val[eval::compute_material_factor_key({1,1,1,0})]=4;
		ret_val[eval::compute_material_factor_key({1,1,1,1})]=19;
		ret_val[eval::compute_material_factor_key({1,1,2,0})]=12;
		ret_val[eval::compute_material_factor_key({1,1,2,1})]=38;
		ret_val[eval::compute_material_factor_key({1,2,0,0})]=65;
		ret_val[eval::compute_material_factor_key({1,2,0,1})]=-31;
		ret_val[eval::compute_material_factor_key({1,2,1,0})]=9;
		ret_val[eval::compute_material_factor_key({1,2,1,1})]=45;
		ret_val[eval::compute_material_factor_key({1,2,2,0})]=19;
		ret_val[eval::compute_material_factor_key({1,2,2,1})]=39;
		ret_val[eval::compute_material_factor_key({2,0,0,0})]=22;
		ret_val[eval::compute_material_factor_key({2,0,0,1})]=-57;
		ret_val[eval::compute_material_factor_key({2,0,1,0})]=7;
		ret_val[eval::compute_material_factor_key({2,0,1,1})]=22;
		ret_val[eval::compute_material_factor_key({2,0,2,0})]=18;
		ret_val[eval::compute_material_factor_key({2,0,2,1})]=33;
		ret_val[eval::compute_material_factor_key({2,1,0,0})]=56;
		ret_val[eval::compute_material_factor_key({2,1,0,1})]=-26;
		ret_val[eval::compute_material_factor_key({2,1,1,0})]=3;
		ret_val[eval::compute_material_factor_key({2,1,1,1})]=25;
		ret_val[eval::compute_material_factor_key({2,1,2,0})]=16;
		ret_val[eval::compute_material_factor_key({2,1,2,1})]=39;
		ret_val[eval::compute_material_factor_key({2,2,0,0})]=1;
		ret_val[eval::compute_material_factor_key({2,2,0,1})]=126;
		ret_val[eval::compute_material_factor_key({2,2,1,0})]=1;
		ret_val[eval::compute_material_factor_key({2,2,1,1})]=39;
		ret_val[eval::compute_material_factor_key({2,2,2,0})]=18;
		ret_val[eval::compute_material_factor_key({2,2,2,1})]=38;

		return ret_val;
	}
	
	constexpr auto init_kingsquare_attacker_table() noexcept
	{
		eval::material_factors<int> ret_val{};
		ret_val[eval::compute_material_factor_key({0,0,0,0})]=-11;
		ret_val[eval::compute_material_factor_key({0,0,0,1})]=3;
		ret_val[eval::compute_material_factor_key({0,0,1,0})]=-4;
		ret_val[eval::compute_material_factor_key({0,0,1,1})]=19;
		ret_val[eval::compute_material_factor_key({0,0,2,0})]=1;
		ret_val[eval::compute_material_factor_key({0,0,2,1})]=27;
		ret_val[eval::compute_material_factor_key({0,1,0,0})]=-4;
		ret_val[eval::compute_material_factor_key({0,1,0,1})]=17;
		ret_val[eval::compute_material_factor_key({0,1,1,0})]=-1;
		ret_val[eval::compute_material_factor_key({0,1,1,1})]=41;
		ret_val[eval::compute_material_factor_key({0,1,2,0})]=6;
		ret_val[eval::compute_material_factor_key({0,1,2,1})]=90;
		ret_val[eval::compute_material_factor_key({0,2,0,0})]=-4;
		ret_val[eval::compute_material_factor_key({0,2,0,1})]=29;
		ret_val[eval::compute_material_factor_key({0,2,1,0})]=6;
		ret_val[eval::compute_material_factor_key({0,2,1,1})]=-36;
		ret_val[eval::compute_material_factor_key({0,2,2,0})]=92;
		ret_val[eval::compute_material_factor_key({0,2,2,1})]=25;
		ret_val[eval::compute_material_factor_key({1,0,0,0})]=-3;
		ret_val[eval::compute_material_factor_key({1,0,0,1})]=14;
		ret_val[eval::compute_material_factor_key({1,0,1,0})]=0;
		ret_val[eval::compute_material_factor_key({1,0,1,1})]=37;
		ret_val[eval::compute_material_factor_key({1,0,2,0})]=9;
		ret_val[eval::compute_material_factor_key({1,0,2,1})]=2;
		ret_val[eval::compute_material_factor_key({1,1,0,0})]=-3;
		ret_val[eval::compute_material_factor_key({1,1,0,1})]=35;
		ret_val[eval::compute_material_factor_key({1,1,1,0})]=2;
		ret_val[eval::compute_material_factor_key({1,1,1,1})]=34;
		ret_val[eval::compute_material_factor_key({1,1,2,0})]=22;
		ret_val[eval::compute_material_factor_key({1,1,2,1})]=33;
		ret_val[eval::compute_material_factor_key({1,2,0,0})]=4;
		ret_val[eval::compute_material_factor_key({1,2,0,1})]=56;
		ret_val[eval::compute_material_factor_key({1,2,1,0})]=-5;
		ret_val[eval::compute_material_factor_key({1,2,1,1})]=25;
		ret_val[eval::compute_material_factor_key({1,2,2,0})]=25;
		ret_val[eval::compute_material_factor_key({1,2,2,1})]=25;
		ret_val[eval::compute_material_factor_key({2,0,0,0})]=0;
		ret_val[eval::compute_material_factor_key({2,0,0,1})]=32;
		ret_val[eval::compute_material_factor_key({2,0,1,0})]=10;
		ret_val[eval::compute_material_factor_key({2,0,1,1})]=25;
		ret_val[eval::compute_material_factor_key({2,0,2,0})]=34;
		ret_val[eval::compute_material_factor_key({2,0,2,1})]=25;
		ret_val[eval::compute_material_factor_key({2,1,0,0})]=4;
		ret_val[eval::compute_material_factor_key({2,1,0,1})]=56;
		ret_val[eval::compute_material_factor_key({2,1,1,0})]=25;
		ret_val[eval::compute_material_factor_key({2,1,1,1})]=176;
		ret_val[eval::compute_material_factor_key({2,1,2,0})]=178;
		ret_val[eval::compute_material_factor_key({2,1,2,1})]=25;
		ret_val[eval::compute_material_factor_key({2,2,0,0})]=16;
		ret_val[eval::compute_material_factor_key({2,2,0,1})]=25;
		ret_val[eval::compute_material_factor_key({2,2,1,0})]=62;
		ret_val[eval::compute_material_factor_key({2,2,1,1})]=25;
		ret_val[eval::compute_material_factor_key({2,2,2,0})]=25;
		ret_val[eval::compute_material_factor_key({2,2,2,1})]=25;

		return ret_val;
	}
		
	constexpr auto init_primary_pawnshield_table() noexcept
	{
		eval::material_factors<int> ret_val{};
		ret_val[eval::compute_material_factor_key({0,0,0,0})]=15;
		ret_val[eval::compute_material_factor_key({0,0,0,1})]=-5;
		ret_val[eval::compute_material_factor_key({0,0,1,0})]=-2;
		ret_val[eval::compute_material_factor_key({0,0,1,1})]=2;
		ret_val[eval::compute_material_factor_key({0,0,2,0})]=9;
		ret_val[eval::compute_material_factor_key({0,0,2,1})]=8;
		ret_val[eval::compute_material_factor_key({0,1,0,0})]=-5;
		ret_val[eval::compute_material_factor_key({0,1,0,1})]=9;
		ret_val[eval::compute_material_factor_key({0,1,1,0})]=-1;
		ret_val[eval::compute_material_factor_key({0,1,1,1})]=10;
		ret_val[eval::compute_material_factor_key({0,1,2,0})]=0;
		ret_val[eval::compute_material_factor_key({0,1,2,1})]=12;
		ret_val[eval::compute_material_factor_key({0,2,0,0})]=8;
		ret_val[eval::compute_material_factor_key({0,2,0,1})]=24;
		ret_val[eval::compute_material_factor_key({0,2,1,0})]=3;
		ret_val[eval::compute_material_factor_key({0,2,1,1})]=27;
		ret_val[eval::compute_material_factor_key({0,2,2,0})]=9;
		ret_val[eval::compute_material_factor_key({0,2,2,1})]=18;
		ret_val[eval::compute_material_factor_key({1,0,0,0})]=2;
		ret_val[eval::compute_material_factor_key({1,0,0,1})]=14;
		ret_val[eval::compute_material_factor_key({1,0,1,0})]=0;
		ret_val[eval::compute_material_factor_key({1,0,1,1})]=15;
		ret_val[eval::compute_material_factor_key({1,0,2,0})]=0;
		ret_val[eval::compute_material_factor_key({1,0,2,1})]=14;
		ret_val[eval::compute_material_factor_key({1,1,0,0})]=12;
		ret_val[eval::compute_material_factor_key({1,1,0,1})]=28;
		ret_val[eval::compute_material_factor_key({1,1,1,0})]=5;
		ret_val[eval::compute_material_factor_key({1,1,1,1})]=24;
		ret_val[eval::compute_material_factor_key({1,1,2,0})]=2;
		ret_val[eval::compute_material_factor_key({1,1,2,1})]=18;
		ret_val[eval::compute_material_factor_key({1,2,0,0})]=17;
		ret_val[eval::compute_material_factor_key({1,2,0,1})]=37;
		ret_val[eval::compute_material_factor_key({1,2,1,0})]=7;
		ret_val[eval::compute_material_factor_key({1,2,1,1})]=35;
		ret_val[eval::compute_material_factor_key({1,2,2,0})]=0;
		ret_val[eval::compute_material_factor_key({1,2,2,1})]=12;
		ret_val[eval::compute_material_factor_key({2,0,0,0})]=0;
		ret_val[eval::compute_material_factor_key({2,0,0,1})]=21;
		ret_val[eval::compute_material_factor_key({2,0,1,0})]=-2;
		ret_val[eval::compute_material_factor_key({2,0,1,1})]=18;
		ret_val[eval::compute_material_factor_key({2,0,2,0})]=-7;
		ret_val[eval::compute_material_factor_key({2,0,2,1})]=8;
		ret_val[eval::compute_material_factor_key({2,1,0,0})]=6;
		ret_val[eval::compute_material_factor_key({2,1,0,1})]=26;
		ret_val[eval::compute_material_factor_key({2,1,1,0})]=-1;
		ret_val[eval::compute_material_factor_key({2,1,1,1})]=24;
		ret_val[eval::compute_material_factor_key({2,1,2,0})]=-7;
		ret_val[eval::compute_material_factor_key({2,1,2,1})]=4;
		ret_val[eval::compute_material_factor_key({2,2,0,0})]=13;
		ret_val[eval::compute_material_factor_key({2,2,0,1})]=19;
		ret_val[eval::compute_material_factor_key({2,2,1,0})]=5;
		ret_val[eval::compute_material_factor_key({2,2,1,1})]=14;
		ret_val[eval::compute_material_factor_key({2,2,2,0})]=-6;
		ret_val[eval::compute_material_factor_key({2,2,2,1})]=0;

		return ret_val;
	}
		
	constexpr auto init_secondary_pawnshield_table() noexcept
	{
		eval::material_factors<int> ret_val{};
		ret_val[eval::compute_material_factor_key({0,0,0,0})]=9;
		ret_val[eval::compute_material_factor_key({0,0,0,1})]=-6;
		ret_val[eval::compute_material_factor_key({0,0,1,0})]=-2;
		ret_val[eval::compute_material_factor_key({0,0,1,1})]=-4;
		ret_val[eval::compute_material_factor_key({0,0,2,0})]=-1;
		ret_val[eval::compute_material_factor_key({0,0,2,1})]=7;
		ret_val[eval::compute_material_factor_key({0,1,0,0})]=-7;
		ret_val[eval::compute_material_factor_key({0,1,0,1})]=0;
		ret_val[eval::compute_material_factor_key({0,1,1,0})]=2;
		ret_val[eval::compute_material_factor_key({0,1,1,1})]=3;
		ret_val[eval::compute_material_factor_key({0,1,2,0})]=-1;
		ret_val[eval::compute_material_factor_key({0,1,2,1})]=6;
		ret_val[eval::compute_material_factor_key({0,2,0,0})]=3;
		ret_val[eval::compute_material_factor_key({0,2,0,1})]=5;
		ret_val[eval::compute_material_factor_key({0,2,1,0})]=6;
		ret_val[eval::compute_material_factor_key({0,2,1,1})]=0;
		ret_val[eval::compute_material_factor_key({0,2,2,0})]=0;
		ret_val[eval::compute_material_factor_key({0,2,2,1})]=8;
		ret_val[eval::compute_material_factor_key({1,0,0,0})]=0;
		ret_val[eval::compute_material_factor_key({1,0,0,1})]=3;
		ret_val[eval::compute_material_factor_key({1,0,1,0})]=6;
		ret_val[eval::compute_material_factor_key({1,0,1,1})]=7;
		ret_val[eval::compute_material_factor_key({1,0,2,0})]=5;
		ret_val[eval::compute_material_factor_key({1,0,2,1})]=6;
		ret_val[eval::compute_material_factor_key({1,1,0,0})]=14;
		ret_val[eval::compute_material_factor_key({1,1,0,1})]=14;
		ret_val[eval::compute_material_factor_key({1,1,1,0})]=9;
		ret_val[eval::compute_material_factor_key({1,1,1,1})]=19;
		ret_val[eval::compute_material_factor_key({1,1,2,0})]=6;
		ret_val[eval::compute_material_factor_key({1,1,2,1})]=10;
		ret_val[eval::compute_material_factor_key({1,2,0,0})]=10;
		ret_val[eval::compute_material_factor_key({1,2,0,1})]=31;
		ret_val[eval::compute_material_factor_key({1,2,1,0})]=17;
		ret_val[eval::compute_material_factor_key({1,2,1,1})]=16;
		ret_val[eval::compute_material_factor_key({1,2,2,0})]=1;
		ret_val[eval::compute_material_factor_key({1,2,2,1})]=5;
		ret_val[eval::compute_material_factor_key({2,0,0,0})]=4;
		ret_val[eval::compute_material_factor_key({2,0,0,1})]=8;
		ret_val[eval::compute_material_factor_key({2,0,1,0})]=0;
		ret_val[eval::compute_material_factor_key({2,0,1,1})]=1;
		ret_val[eval::compute_material_factor_key({2,0,2,0})]=-5;
		ret_val[eval::compute_material_factor_key({2,0,2,1})]=-2;
		ret_val[eval::compute_material_factor_key({2,1,0,0})]=15;
		ret_val[eval::compute_material_factor_key({2,1,0,1})]=13;
		ret_val[eval::compute_material_factor_key({2,1,1,0})]=-2;
		ret_val[eval::compute_material_factor_key({2,1,1,1})]=4;
		ret_val[eval::compute_material_factor_key({2,1,2,0})]=-3;
		ret_val[eval::compute_material_factor_key({2,1,2,1})]=-3;
		ret_val[eval::compute_material_factor_key({2,2,0,0})]=51;
		ret_val[eval::compute_material_factor_key({2,2,0,1})]=62;
		ret_val[eval::compute_material_factor_key({2,2,1,0})]=4;
		ret_val[eval::compute_material_factor_key({2,2,1,1})]=2;
		ret_val[eval::compute_material_factor_key({2,2,2,0})]=-7;
		ret_val[eval::compute_material_factor_key({2,2,2,1})]=-4;

		return ret_val;
	}
	
	constexpr auto init_open_files_table() noexcept
	{
		eval::material_factors<int> ret_val{};
		ret_val[eval::compute_material_factor_key({0,0,0,0})]=-6;
		ret_val[eval::compute_material_factor_key({0,0,0,1})]=-6;
		ret_val[eval::compute_material_factor_key({0,0,1,0})]=-9;
		ret_val[eval::compute_material_factor_key({0,0,1,1})]=-24;
		ret_val[eval::compute_material_factor_key({0,0,2,0})]=-2;
		ret_val[eval::compute_material_factor_key({0,0,2,1})]=-26;
		ret_val[eval::compute_material_factor_key({0,1,0,0})]=1;
		ret_val[eval::compute_material_factor_key({0,1,0,1})]=-1;
		ret_val[eval::compute_material_factor_key({0,1,1,0})]=4;
		ret_val[eval::compute_material_factor_key({0,1,1,1})]=-18;
		ret_val[eval::compute_material_factor_key({0,1,2,0})]=-9;
		ret_val[eval::compute_material_factor_key({0,1,2,1})]=-30;
		ret_val[eval::compute_material_factor_key({0,2,0,0})]=18;
		ret_val[eval::compute_material_factor_key({0,2,0,1})]=-2;
		ret_val[eval::compute_material_factor_key({0,2,1,0})]=0;
		ret_val[eval::compute_material_factor_key({0,2,1,1})]=-14;
		ret_val[eval::compute_material_factor_key({0,2,2,0})]=-11;
		ret_val[eval::compute_material_factor_key({0,2,2,1})]=-18;
		ret_val[eval::compute_material_factor_key({1,0,0,0})]=6;
		ret_val[eval::compute_material_factor_key({1,0,0,1})]=5;
		ret_val[eval::compute_material_factor_key({1,0,1,0})]=9;
		ret_val[eval::compute_material_factor_key({1,0,1,1})]=-16;
		ret_val[eval::compute_material_factor_key({1,0,2,0})]=-6;
		ret_val[eval::compute_material_factor_key({1,0,2,1})]=-18;
		ret_val[eval::compute_material_factor_key({1,1,0,0})]=17;
		ret_val[eval::compute_material_factor_key({1,1,0,1})]=6;
		ret_val[eval::compute_material_factor_key({1,1,1,0})]=4;
		ret_val[eval::compute_material_factor_key({1,1,1,1})]=-9;
		ret_val[eval::compute_material_factor_key({1,1,2,0})]=-7;
		ret_val[eval::compute_material_factor_key({1,1,2,1})]=-19;
		ret_val[eval::compute_material_factor_key({1,2,0,0})]=21;
		ret_val[eval::compute_material_factor_key({1,2,0,1})]=-7;
		ret_val[eval::compute_material_factor_key({1,2,1,0})]=7;
		ret_val[eval::compute_material_factor_key({1,2,1,1})]=-10;
		ret_val[eval::compute_material_factor_key({1,2,2,0})]=-17;
		ret_val[eval::compute_material_factor_key({1,2,2,1})]=-13;
		ret_val[eval::compute_material_factor_key({2,0,0,0})]=2;
		ret_val[eval::compute_material_factor_key({2,0,0,1})]=-11;
		ret_val[eval::compute_material_factor_key({2,0,1,0})]=-10;
		ret_val[eval::compute_material_factor_key({2,0,1,1})]=-36;
		ret_val[eval::compute_material_factor_key({2,0,2,0})]=-24;
		ret_val[eval::compute_material_factor_key({2,0,2,1})]=-37;
		ret_val[eval::compute_material_factor_key({2,1,0,0})]=5;
		ret_val[eval::compute_material_factor_key({2,1,0,1})]=-13;
		ret_val[eval::compute_material_factor_key({2,1,1,0})]=-8;
		ret_val[eval::compute_material_factor_key({2,1,1,1})]=-17;
		ret_val[eval::compute_material_factor_key({2,1,2,0})]=-28;
		ret_val[eval::compute_material_factor_key({2,1,2,1})]=-23;
		ret_val[eval::compute_material_factor_key({2,2,0,0})]=-7;
		ret_val[eval::compute_material_factor_key({2,2,0,1})]=-48;
		ret_val[eval::compute_material_factor_key({2,2,1,0})]=-28;
		ret_val[eval::compute_material_factor_key({2,2,1,1})]=-8;
		ret_val[eval::compute_material_factor_key({2,2,2,0})]=-30;
		ret_val[eval::compute_material_factor_key({2,2,2,1})]=-28;

		return ret_val;
	}	
}

const eval::material_factors<int>& eval::get_castling_rights_table() noexcept
{
	const static auto tables=init_castling_rights_table();
	return tables;
}

const eval::material_factors<int>& eval::get_kingsquare_attacker_table() noexcept
{
	const static auto tables=init_kingsquare_attacker_table();
	return tables;
}

const eval::material_factors<int>& eval::get_primary_pawnshield_table() noexcept
{
	const static auto tables=init_primary_pawnshield_table();
	return tables;
}

const eval::material_factors<int>& eval::get_secondary_pawnshield_table() noexcept
{
	const static auto tables=init_secondary_pawnshield_table();
	return tables;
}

const eval::material_factors<int>& eval::get_open_files_table() noexcept
{
	const static auto tables=init_open_files_table();
	return tables;
}
