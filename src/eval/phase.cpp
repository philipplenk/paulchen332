#include <philchess/eval/phase.hpp>

using namespace philchess;

namespace
{
	constexpr auto init_phase_factor_table()
	{
		eval::material_factors<int> ret_val{};
		ret_val[eval::compute_material_factor_key({0,0,0,0})]=188;
		ret_val[eval::compute_material_factor_key({0,0,0,1})]=263;
		ret_val[eval::compute_material_factor_key({0,0,1,0})]=188;
		ret_val[eval::compute_material_factor_key({0,0,1,1})]=255;
		ret_val[eval::compute_material_factor_key({0,0,2,0})]=246;
		ret_val[eval::compute_material_factor_key({0,0,2,1})]=286;
		ret_val[eval::compute_material_factor_key({0,1,0,0})]=181;
		ret_val[eval::compute_material_factor_key({0,1,0,1})]=244;
		ret_val[eval::compute_material_factor_key({0,1,1,0})]=225;
		ret_val[eval::compute_material_factor_key({0,1,1,1})]=275;
		ret_val[eval::compute_material_factor_key({0,1,2,0})]=233;
		ret_val[eval::compute_material_factor_key({0,1,2,1})]=281;
		ret_val[eval::compute_material_factor_key({0,2,0,0})]=227;
		ret_val[eval::compute_material_factor_key({0,2,0,1})]=271;
		ret_val[eval::compute_material_factor_key({0,2,1,0})]=232;
		ret_val[eval::compute_material_factor_key({0,2,1,1})]=299;
		ret_val[eval::compute_material_factor_key({0,2,2,0})]=255;
		ret_val[eval::compute_material_factor_key({0,2,2,1})]=295;
		ret_val[eval::compute_material_factor_key({1,0,0,0})]=203;
		ret_val[eval::compute_material_factor_key({1,0,0,1})]=252;
		ret_val[eval::compute_material_factor_key({1,0,1,0})]=230;
		ret_val[eval::compute_material_factor_key({1,0,1,1})]=284;
		ret_val[eval::compute_material_factor_key({1,0,2,0})]=242;
		ret_val[eval::compute_material_factor_key({1,0,2,1})]=286;
		ret_val[eval::compute_material_factor_key({1,1,0,0})]=234;
		ret_val[eval::compute_material_factor_key({1,1,0,1})]=266;
		ret_val[eval::compute_material_factor_key({1,1,1,0})]=238;
		ret_val[eval::compute_material_factor_key({1,1,1,1})]=296;
		ret_val[eval::compute_material_factor_key({1,1,2,0})]=252;
		ret_val[eval::compute_material_factor_key({1,1,2,1})]=298;
		ret_val[eval::compute_material_factor_key({1,2,0,0})]=240;
		ret_val[eval::compute_material_factor_key({1,2,0,1})]=278;
		ret_val[eval::compute_material_factor_key({1,2,1,0})]=223;
		ret_val[eval::compute_material_factor_key({1,2,1,1})]=289;
		ret_val[eval::compute_material_factor_key({1,2,2,0})]=263;
		ret_val[eval::compute_material_factor_key({1,2,2,1})]=300;
		ret_val[eval::compute_material_factor_key({2,0,0,0})]=260;
		ret_val[eval::compute_material_factor_key({2,0,0,1})]=273;
		ret_val[eval::compute_material_factor_key({2,0,1,0})]=238;
		ret_val[eval::compute_material_factor_key({2,0,1,1})]=304;
		ret_val[eval::compute_material_factor_key({2,0,2,0})]=256;
		ret_val[eval::compute_material_factor_key({2,0,2,1})]=297;
		ret_val[eval::compute_material_factor_key({2,1,0,0})]=236;
		ret_val[eval::compute_material_factor_key({2,1,0,1})]=268;
		ret_val[eval::compute_material_factor_key({2,1,1,0})]=240;
		ret_val[eval::compute_material_factor_key({2,1,1,1})]=289;
		ret_val[eval::compute_material_factor_key({2,1,2,0})]=262;
		ret_val[eval::compute_material_factor_key({2,1,2,1})]=301;
		ret_val[eval::compute_material_factor_key({2,2,0,0})]=248;
		ret_val[eval::compute_material_factor_key({2,2,0,1})]=282;
		ret_val[eval::compute_material_factor_key({2,2,1,0})]=245;
		ret_val[eval::compute_material_factor_key({2,2,1,1})]=300;
		ret_val[eval::compute_material_factor_key({2,2,2,0})]=268;
		ret_val[eval::compute_material_factor_key({2,2,2,1})]=314;

		return ret_val;
	}
}

const eval::material_factors<int>& eval::get_phase_factor_table()
{
	const static auto tables=init_phase_factor_table();
	return tables;
}
