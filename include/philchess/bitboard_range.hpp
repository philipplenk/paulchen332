#ifndef PHILCHESS_UTILITY_BITBOARD_RANGE_H
#define PHILCHESS_UTILITY_BITBOARD_RANGE_H

#include <philchess/bitboard.hpp>
#include <philchess/types.hpp>

#include <ptl/bit.hpp>

namespace philchess::bitboard
{
	class range
	{
		public:
		class square_iter
		{
			public:
			explicit constexpr square_iter(std::uint64_t ranks) noexcept:
				ranks_{ranks},
				next_{ptl::countr_zero(ranks_)}
			{}
			
			static constexpr auto sentinel() noexcept { return square_iter{0}; }
			
			constexpr auto& operator++() noexcept
			{
				ranks_^=(std::uint64_t{1}<<next_);
				next_ = ptl::countr_zero(ranks_);
				
				return *this;
			}
			constexpr auto operator++(int) noexcept { auto tmp{*this}; ++(*this); return tmp; }
	
			constexpr auto operator*() const noexcept { return square{next_}; }
			constexpr auto operator->() const noexcept { return square{next_}; }
	
			constexpr auto operator==(const square_iter& rhs) noexcept
			{
				return ranks_==rhs.ranks_;
			}
	
			constexpr auto operator!=(const square_iter& rhs) noexcept
			{
				return !(*this==rhs);
			}
			
			private:
			std::uint64_t ranks_;
			std::size_t next_;
		};
		
		constexpr range(bitboard::rank data) noexcept:
			data_{data}
		{}
		
		constexpr auto begin() const noexcept { return square_iter{data_.ranks()}; }
		constexpr auto end() const noexcept { return square_iter::sentinel(); }
		
		private:
		bitboard::rank data_;
	};
	
	constexpr range squares(bitboard::rank data) noexcept { return range{data}; }
} //end namespace philchess::bitboard

#endif
