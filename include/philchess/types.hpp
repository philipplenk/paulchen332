#ifndef PHILCHESS_TYPES_H
#define PHILCHESS_TYPES_H

#include <ptl/enum_map.hpp>

#include <array>
#include <iostream>
#include <type_traits>

#include <cstdint>

namespace philchess
{
	enum class score_type: std::uint8_t
	{
		exact,
		lower_bound,
		upper_bound
	};

	enum class side: std::uint8_t
	{
		white=0, black
	};
	constexpr side reverse(side s) noexcept { return static_cast<side>((static_cast<::std::uint8_t>(s)+1)%2); }
	template <typename T> using side_map=ptl::enum_map<side,T,2>;
	
	enum class castling_right: std::uint8_t
	{
		none=0, kingside=0x1, queenside=0x2, both=0x3
	};
	template <typename T> using castling_right_map=ptl::enum_map<castling_right,T,4>;
	constexpr castling_right operator|(castling_right lhs, castling_right rhs) noexcept { return static_cast<castling_right>(static_cast<::std::uint8_t>(lhs)|static_cast<::std::uint8_t>(rhs)); }
	
	enum class piece_type: std::uint8_t
	{
		pawn, knight, bishop, rook, queen, king, none
	};
	template <typename T> using piece_type_map=ptl::enum_map<piece_type,T,7>;
	
	inline ::std::ostream& operator<<(::std::ostream& out, const piece_type& p)
	{
		constexpr auto names="PNBRQKx";
		
		out<<names[static_cast<std::uint8_t>(p)];
		
		return out;
	}
	
	enum class move_type: std::uint8_t
	{
		normal=0, promotion, en_passant, castling
	};
	
	class square
	{
		public:
		constexpr square() noexcept = default;
		
		template <typename INT_TYPE_T>
		constexpr explicit square(INT_TYPE_T id) noexcept:
			data{static_cast<std::uint8_t>(id)}
		{}
		
		template <typename INT_TYPE0, typename INT_TYPE1>
		constexpr square(INT_TYPE0 file, INT_TYPE1 rank) noexcept:
			data
			{
				static_cast<std::uint8_t>(static_cast<std::uint8_t>(file) | static_cast<std::uint8_t>(rank)<<3)
			}
		{}
		
		constexpr auto rank() const noexcept { return static_cast<std::uint8_t>(data>>3); }
		constexpr auto file() const noexcept { return static_cast<std::uint8_t>(data&0x7); }
		constexpr auto diagonal() const noexcept { return rank()+file(); }
		constexpr auto antidiagonal() const noexcept { return 7+rank()-file(); }
		constexpr auto id() const noexcept { return data; }
		
		friend constexpr auto operator==(square lhs, square rhs) noexcept { return lhs.data==rhs.data; }
		friend constexpr auto operator!=(square lhs, square rhs) noexcept { return !(lhs==rhs); }
		
		private:
		std::uint8_t data{};
	};
	static_assert(std::is_trivially_copyable_v<square>,"You somehow managed to break squares's triviality...");

	
	template <typename T>
	struct square_table
	{
		constexpr auto& operator[](square sq) noexcept { return data[sq.id()]; }
		constexpr const auto& operator[](square sq) const noexcept { return data[sq.id()]; }
		
		constexpr auto begin() noexcept { return data.begin(); }
		constexpr auto end() noexcept { return data.end(); }
		constexpr auto size() const noexcept { return data.size(); }
		
		::std::array<T,64> data{};
	};
	
	class move
	{
		public:
		constexpr move() noexcept:
			move(square{0},square{0})
		{}
		
		constexpr move(square from, square to) noexcept:
			data{ static_cast<std::uint16_t>(
				std::uint16_t{from.id()}<<10 |
				((std::uint16_t{to.id()}<<4)&0x03f0)
			)}
		{}
		
		constexpr move(square from, square to, piece_type promotion) noexcept:
			move(from,to)
		{
			data|=static_cast<std::uint8_t>(move_type::promotion)<<2;
			data|=(static_cast<std::uint8_t>(promotion)-1);
		}
		
		constexpr auto from() const noexcept { return square{static_cast<std::uint8_t>(data>>10)}; }
		constexpr auto to() const noexcept { return square{static_cast<std::uint8_t>((data>>4)&0x3f)}; }
		constexpr auto type() const noexcept { return static_cast<move_type>((data>>2)&0x3); }
		constexpr auto promote_to() const noexcept { return static_cast<piece_type>((data&0x3)+1); }
		
		constexpr auto as_enpassant() const noexcept { auto cpy=*this; cpy.data|=static_cast<std::uint8_t>(move_type::en_passant)<<2; return cpy; } 
		constexpr auto as_castle() const noexcept { auto cpy=*this; cpy.data|=static_cast<std::uint8_t>(move_type::castling)<<2; return cpy; } 
		
		friend constexpr bool operator==(move lhs, move rhs) noexcept { return lhs.data==rhs.data; }
		
		private:
		std::uint16_t data;
	};
	static_assert(std::is_trivially_copyable_v<move>,"You somehow managed to break move's triviality...");
	
	inline ::std::istream& operator>>(::std::istream& in, move& m)
	{
		::std::string val;
		if(!(in>>val))
			return in;
		
		const auto from=square{(val[0]-'a'),(val[1]-'0'-1)};
		const auto to=square{(val[2]-'a'), (val[3]-'0'-1)};
		
		if(val.size()==4)
		{
			m=move{from,to};
		}
		else if(val.size()==5)
		{
			piece_type promote_to=piece_type::queen;
			switch(val[4])
			{
				case 'n': promote_to=piece_type::knight; break;
				case 'b': promote_to=piece_type::bishop; break;
				case 'r': promote_to=piece_type::rook; break;
				case 'q': promote_to=piece_type::queen; break;
				default: in.setstate(std::ios_base::failbit); break; 
			}
			m=move{from,to,promote_to};
		}
		else
			in.setstate(std::ios_base::failbit);
		
		return in;
	}
	
	inline ::std::ostream& operator<<(::std::ostream& out, const move& m)
	{
		constexpr auto letters="abcdefgh";
		
		const auto from=m.from();
		const auto to=m.to();
		
		out<<letters[from.file()]<<(from.rank()+1)<<letters[to.file()]<<(to.rank()+1);
		
		if(m.type()==move_type::promotion)
		{
			constexpr auto piece_letters="pnbrqkx";
			const auto piece=m.promote_to();
			out<<piece_letters[static_cast<std::uint8_t>(piece)];
		}
		
		return out;
	}
	
} //end namespace philchess

#endif
