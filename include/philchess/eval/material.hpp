#ifndef PHILCHESS_EVAL_MATERIAL_H
#define PHILCHESS_EVAL_MATERIAL_H

#include <philchess/types.hpp>

#include <ptl/handle.hpp>

namespace philchess {
namespace eval
{
	inline const auto& get_material_table() noexcept
	{
		const static auto material_table=[]()
		{
			philchess::piece_type_map<int> values;
			values[piece_type::bishop]=3;
			values[piece_type::king]=10000;
			values[piece_type::knight]=3;
			values[piece_type::none]=0;
			values[piece_type::pawn]=1;
			values[piece_type::queen]=9;
			values[piece_type::rook]=5;
			return values;
		}();
		
		return material_table;
	}
	
	struct material_counts_t
	{
		unsigned n_bishops, n_knights, n_rooks, n_queens;
	};
	
	using material_factor_key=ptl::handle<struct material_factor_key_tag, std::size_t>;
	constexpr material_factor_key compute_material_factor_key(material_counts_t counts) noexcept //limit to max starting material. it not really worth considering the cases with additional material, as this usually means a decicive material advantage or some complex tactics, neither of which profits from delicately tuned material scaled table values...
	{
		counts.n_queens=std::min(counts.n_queens,1u);
		counts.n_rooks=std::min(counts.n_rooks,2u);
		counts.n_bishops=std::min(counts.n_bishops,2u);
		counts.n_knights=std::min(counts.n_knights,2u);
		
		return material_factor_key(counts.n_queens+2*counts.n_rooks+2*3*counts.n_bishops+2*3*3*counts.n_knights);
	}
	
	constexpr material_factor_key extract_and_compute_material_factor_key(piece_type_map<unsigned> counts) noexcept
	{
		counts[piece_type::queen]=std::min(counts[piece_type::queen],1u);
		counts[piece_type::rook]=std::min(counts[piece_type::rook],2u);
		counts[piece_type::bishop]=std::min(counts[piece_type::bishop],2u);
		counts[piece_type::knight]=std::min(counts[piece_type::knight],2u);
		
		return material_factor_key(counts[piece_type::queen]+2*counts[piece_type::rook]+2*3*counts[piece_type::bishop]+2*3*3*counts[piece_type::knight]);
	}
	
	template <typename FACTOR_T>
	class material_factors
	{
		public:
		constexpr FACTOR_T& operator[](material_factor_key key) noexcept { return factors[key.underlying()]; }
		constexpr const FACTOR_T& operator[](material_factor_key key) const noexcept { return factors[key.underlying()]; }
		
		private:
		std::array<FACTOR_T, 2*3*3*3> factors{};
	};
	

}} //end namespace philchess::eval

#endif
