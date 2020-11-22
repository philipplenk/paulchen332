#ifndef PHILCHESS_UTILITY_BITBOARD_H
#define PHILCHESS_UTILITY_BITBOARD_H

#include <philchess/types.hpp>

#include <ptl/operators.hpp>

#include <iostream>
#include <type_traits>

namespace philchess::bitboard
{
	class rank_index
	{
		public:
		static constexpr auto from_ranks(std::uint64_t ranks) noexcept { rank_index ret_val; ret_val.data_=ranks; return ret_val; } 
		
		constexpr std::uint8_t rank(std::uint8_t id) const noexcept { return static_cast<std::uint8_t>((data_>>(8*id))&0xff); }
		constexpr auto ranks() const noexcept { return data_; }
		
		protected:
		constexpr void set_impl(square sq) noexcept
		{
			data_|=static_cast<std::uint64_t>(std::uint64_t{1}<<sq.id());
		}
		
		constexpr void unset_impl(square sq) noexcept
		{
			data_&=~static_cast<std::uint64_t>(std::uint64_t{1}<<sq.id());
		}
		
		constexpr auto& operator|=(const rank_index& other) noexcept { data_|=other.data_; return *this; }
		constexpr auto& operator&=(const rank_index& other) noexcept { data_&=other.data_; return *this; }
		constexpr auto& operator^=(const rank_index& other) noexcept { data_^=other.data_; return *this; }
		
		constexpr auto operator~() const noexcept { rank_index ret_val; ret_val.data_=~data_; return ret_val; }
				
		private:
		std::uint64_t data_=0;
	};
	
	class file_index
	{
		public:
		static constexpr auto from_files(std::uint64_t files) noexcept { file_index ret_val; ret_val.data_=files; return ret_val; } 
		static constexpr auto from_file(std::uint8_t file, std::uint8_t file_id) noexcept { file_index ret_val; ret_val.data_=std::uint64_t{file}<<(file_id*8); return ret_val; } 
		
		constexpr std::uint8_t file(std::uint8_t id) const noexcept { return static_cast<std::uint8_t>((data_>>(8*id))&0xff); }
		constexpr auto files() const noexcept { return data_; }
		
		protected:
		constexpr void set_impl(square sq) noexcept
		{
			data_|=static_cast<std::uint64_t>(std::uint64_t{1}<<square{sq.rank(),sq.file()}.id());
		}
		
		constexpr void unset_impl(square sq) noexcept
		{
			data_&=~static_cast<std::uint64_t>(std::uint64_t{1}<<square{sq.rank(),sq.file()}.id());
		}
		
		constexpr auto& operator|=(const file_index& other) noexcept { data_|=other.data_; return *this; }
		constexpr auto& operator&=(const file_index& other) noexcept { data_&=other.data_; return *this; }
		constexpr auto& operator^=(const file_index& other) noexcept { data_^=other.data_; return *this; }
				
		constexpr auto operator~() const noexcept { file_index ret_val; ret_val.data_=~data_; return ret_val; }		
		
		private:
		std::uint64_t data_=0;
	};
	
	class diagonal_index
	{
		public:
		constexpr std::uint8_t diagonal(std::uint8_t id) const noexcept //ugly as hell for now...
		{
			constexpr const ::std::array<::std::uint8_t,15> masks
			{{
				0b1,0b11,0b111,0b1111,0b11111,0b111111,0b1111111,0b11111111,0b1111111,0b111111,0b11111,0b1111,0b111,0b11,0b1
			}};
			
			::std::uint8_t ret_val=0;
			const auto idx=id%8;
			const auto shift=id>=8?idx+1:0;
			
			ret_val=static_cast<std::uint8_t>((data_>>(8*idx+shift))&masks[id]);
			
			return ret_val;
		}
		
		constexpr std::uint8_t file_indexed_unmasked_diagonal(std::uint8_t id) const noexcept //ugly as hell for now...
		{
			return static_cast<std::uint8_t>((data_>>(8*(id%8)))&0xff);
		}
		
		constexpr auto diagonals() const noexcept { return data_; }
		
		protected:
		constexpr void set_impl(square sq) noexcept
		{
			auto d_id=sq.rank()+sq.file();
			auto d_idx=d_id%8;
			data_|=static_cast<std::uint64_t>(std::uint64_t{1}<<(8*d_idx+sq.file()));
		}
		
		constexpr void unset_impl(square sq) noexcept
		{
			auto d_id=sq.rank()+sq.file();
			auto d_idx=d_id%8;
			data_&=~static_cast<std::uint64_t>(std::uint64_t{1}<<(8*d_idx+sq.file()));
		}
		
		constexpr auto& operator|=(const diagonal_index& other) noexcept { data_|=other.data_; return *this; }
		constexpr auto& operator&=(const diagonal_index& other) noexcept { data_&=other.data_; return *this; }
		constexpr auto& operator^=(const diagonal_index& other) noexcept { data_^=other.data_; return *this; }
		
		constexpr auto operator~() const noexcept { diagonal_index ret_val; ret_val.data_=~data_; return ret_val; }		
		
		private:
		std::uint64_t data_=0;
	};
	
	class antidiagonal_index
	{
		public:
		constexpr std::uint8_t anti_diagonal(std::uint8_t id) const noexcept //ugly as hell for now...
		{
			constexpr const ::std::array<::std::uint8_t,15> masks
			{{
				0b1,0b11,0b111,0b1111,0b11111,0b111111,0b1111111,0b11111111,0b1111111,0b111111,0b11111,0b1111,0b111,0b11,0b1
			}};
	
			::std::uint8_t ret_val=0;
			const auto idx=id%8;
			const auto shift=id<8?7-idx:0;
			
			ret_val=static_cast<std::uint8_t>((data_>>(8*idx+shift))&masks[id]);
			
			return ret_val;
		}
		
		constexpr std::uint8_t file_indexed_unmasked_anti_diagonal(std::uint8_t id) const noexcept //ugly as hell for now...
		{
			return static_cast<std::uint8_t>((data_>>(8*(id%8)))&0xff);
		}
		
		constexpr auto anti_diagonals() const noexcept { return data_; }
		
		protected:
		constexpr void set_impl(square sq) noexcept
		{
			auto ad_id=7+sq.rank()-sq.file();
			auto ad_idx=ad_id%8;
			data_|=static_cast<std::uint64_t>(std::uint64_t{1}<<(8*ad_idx+sq.file()));
		}
		
		constexpr void unset_impl(square sq) noexcept
		{
			auto ad_id=7+sq.rank()-sq.file();
			auto ad_idx=ad_id%8;
			data_&=~static_cast<std::uint64_t>(std::uint64_t{1}<<(8*ad_idx+sq.file()));
		}
		
		constexpr auto& operator|=(const antidiagonal_index& other) noexcept { data_|=other.data_; return *this; }
		constexpr auto& operator&=(const antidiagonal_index& other) noexcept { data_&=other.data_; return *this; }
		constexpr auto& operator^=(const antidiagonal_index& other) noexcept { data_^=other.data_; return *this; }

		constexpr auto operator~() const noexcept { antidiagonal_index ret_val; ret_val.data_=~data_; return ret_val; }		
				
		private:
		std::uint64_t data_=0;
	};
	
	
	template <typename... IndexTs>
	class composed: public IndexTs..., ptl::operators::bitwise<composed<IndexTs...>>
	{
		public:
		constexpr composed() noexcept = default;
		
		constexpr composed(const IndexTs&... parts) noexcept: //we really want implicit conversions here if possible
			IndexTs{parts}...
		{}
		
		constexpr void set(square sq) noexcept
		{
			auto unused={(IndexTs::set_impl(sq),true)...};
			(void)unused;
		}
		
		constexpr void unset(square sq) noexcept
		{
			auto unused={(IndexTs::unset_impl(sq),true)...};
			(void)unused;
		}
		
		constexpr auto& operator|=(const composed& other) noexcept
		{
			auto unused={(IndexTs::operator|=(other),true)...};
			(void)unused;
			return *this;
		}
		
		constexpr auto& operator&=(const composed& other) noexcept
		{
			auto unused={(IndexTs::operator&=(other),true)...};
			(void)unused;
			return *this;
		}
		
		constexpr auto& operator^=(const composed& other) noexcept
		{
			auto unused={(IndexTs::operator^=(other),true)...};
			(void)unused;
			return *this;
		}
		
		constexpr auto operator~() const noexcept
		{
			return composed{IndexTs::operator~()...};
		}
	};
	
	using rank=composed<rank_index>;
	using file=composed<file_index>;
	using diagonal=composed<diagonal_index>;
	using antidiagonal=composed<antidiagonal_index>;
	using all=composed<file_index,rank_index,diagonal_index,antidiagonal_index>;
	
	inline auto& operator<<(std::ostream& out, const rank_index& ranks)
	{
		out<<"Ranks:\n";
		for(int r=7; r>=0; --r)
		{
			for(int f=0; f<8; ++f)
			{
				out<<(static_cast<int>(ranks.rank(r)>>f)&1)<<' ';
			}
			out<<'\n';
		}
		return out;
	}
	
	inline auto& operator<<(std::ostream& out, const file_index& files)
	{
		out<<"Files:\n";
		for(int r=7; r>=0; --r)
		{
			for(int f=0; f<8; ++f)
			{
				out<<(static_cast<int>(files.file(f)>>r)&1)<<' ';
			}
			out<<'\n';
		}
		return out;
	}
	
	inline auto& operator<<(std::ostream& out, const diagonal_index& diagonals)
	{
		out<<"Diagonals:\n";
		for(int r=7; r>=0; --r)
		{
			for(int f=0; f<8; ++f)
			{
				out<<(static_cast<int>(diagonals.file_indexed_unmasked_diagonal(square{f,r}.diagonal())>>f)&1)<<' ';
			}
			out<<'\n';
		}
		return out;
	}
	
	inline auto& operator<<(std::ostream& out, const antidiagonal_index& diagonals)
	{
		out<<"Antidiagonals:\n";
		for(int r=7; r>=0; --r)
		{
			for(int f=0; f<8; ++f)
			{
				out<<(static_cast<int>(diagonals.file_indexed_unmasked_anti_diagonal(square{f,r}.antidiagonal())>>f)&1)<<' ';
			}
			out<<'\n';
		}
		return out;
	}
	
	template <typename... IndexTs>
	inline auto& operator<<(std::ostream& out, const composed<IndexTs...>& composed_bb)
	{
		out<<"Composed:\n";
		
		auto unused={(out<<static_cast<const IndexTs&>(composed_bb),true)...};
		(void)unused;
		
		return out;
	}
	
} //end namespace philchess::bitboard

#endif
