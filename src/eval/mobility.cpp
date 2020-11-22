#include <philchess/eval/mobility.hpp>

using namespace philchess;

namespace
{
	constexpr auto init_mobility_eval_tables() noexcept //wastes ram and precious cache space for none, king and pawn, which are not used
	{
		piece_type_map<eval::mobility_eval_table<int>> ret_val{};
		ret_val[piece_type::bishop] = { -33, -21, -14, -8, -3, 1, 6, 8, 11, 11, 12, 13, 8, 16, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, };
		ret_val[piece_type::knight] = { -11, -1, 2, 2, 4, 3, 2, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, };
		ret_val[piece_type::rook] = { -27, -22, -18, -13, -10, -6, -1, 1, 5, 8, 12, 13, 18, 22, 21, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, };
		ret_val[piece_type::queen] = { -12, -19, -19, -20, -15, -11, -14, -12, -12, -9, -7, -6, -7, -1, 1, 1, 4, 9, 15, 17, 21, 25, 32, 32, 29, 39, 22, 33, };

		return ret_val;
	}
}

const piece_type_map<eval::mobility_eval_table<int>>& philchess::eval::get_mobility_eval_tables() noexcept
{
	const static auto ret_val = init_mobility_eval_tables();
	return ret_val;
}
