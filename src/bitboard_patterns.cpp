#include <philchess/bitboard_patterns.hpp>

using namespace philchess;

square_table<bitboard::rank> philchess::detail::compute_knight_attacks() noexcept
{
	square_table<bitboard::rank> ret_val;
	
	for(std::uint_fast8_t i=0;i<64;++i)
	{
		square sq{i};
		
		if(sq.file()+2<8 && sq.rank()+1<8)
			ret_val[sq].set(square{sq.file()+2,sq.rank()+1});
			
		if(sq.file()+2<8 && sq.rank()-1>=0)
			ret_val[sq].set(square{sq.file()+2,sq.rank()-1});
			
		if(sq.file()-2>=0 && sq.rank()+1<8)
			ret_val[sq].set(square{sq.file()-2,sq.rank()+1});
			
		if(sq.file()-2>=0 && sq.rank()-1>=0)
			ret_val[sq].set(square{sq.file()-2,sq.rank()-1});

		if(sq.file()+1<8 && sq.rank()+2<8)
			ret_val[sq].set(square{sq.file()+1,sq.rank()+2});
			
		if(sq.file()+1<8 && sq.rank()-2>=0)
			ret_val[sq].set(square{sq.file()+1,sq.rank()-2});
			
		if(sq.file()-1>=0 && sq.rank()+2<8)
			ret_val[sq].set(square{sq.file()-1,sq.rank()+2});
			
		if(sq.file()-1>=0 && sq.rank()-2>=0)
			ret_val[sq].set(square{sq.file()-1,sq.rank()-2});
		
	};
	
	return ret_val;
}

square_table<bitboard::rank> philchess::detail::compute_king_attacks() noexcept
{
	square_table<bitboard::rank> ret_val;
	
	for(std::uint8_t i=0;i<64;++i)
	{
		square sq{i};
		
		if(sq.rank()+1<8) ret_val[sq].set(square{sq.file(),sq.rank()+1});
		if(sq.file()+1<8 && sq.rank()+1<8) ret_val[sq].set(square{sq.file()+1,sq.rank()+1});
		if(sq.file()-1>=0 && sq.rank()+1<8) ret_val[sq].set(square{sq.file()-1,sq.rank()+1});
		if(sq.rank()-1>=0) ret_val[sq].set(square{sq.file(),sq.rank()-1});
		if(sq.file()+1<8 && sq.rank()-1>=0) ret_val[sq].set(square{sq.file()+1,sq.rank()-1});
		if(sq.file()-1>=0 && sq.rank()-1>=0) ret_val[sq].set(square{sq.file()-1,sq.rank()-1});
		if(sq.file()+1<8) ret_val[sq].set(square{sq.file()+1,sq.rank()});
		if(sq.file()-1>=0) ret_val[sq].set(square{sq.file()-1,sq.rank()});
	};
	
	return ret_val;
}

square_table<std::array<bitboard::rank,256>> philchess::detail::compute_rook_rank_attacks() noexcept
{
	square_table<std::array<bitboard::rank,256>> ret_val;
	
	for(std::uint_fast8_t id=0;id<64;++id)
	{
		const auto sq=square{id};
		
		for(std::uint_fast16_t occupancy=0;occupancy<256;++occupancy)
		{
			for(int r=sq.rank()+1;r<8;++r)
			{
				ret_val[sq][occupancy].set(square(sq.file(),r));
				if((occupancy>>r)&1)
					break;
			}
			for(int r=sq.rank()-1;r>=0;--r)
			{
				ret_val[sq][occupancy].set(square(sq.file(),r));
				if((occupancy>>r)&1)
					break;
			}
		}
	}
	
	return ret_val;
}

square_table<std::array<bitboard::rank,256>> philchess::detail::compute_rook_file_attacks() noexcept
{
	square_table<std::array<bitboard::rank,256>> ret_val;
	
	for(std::uint_fast8_t id=0;id<64;++id)
	{
		const auto sq=square{id};
		
		for(std::uint_fast16_t occupancy=0;occupancy<256;++occupancy)
		{
			for(int f=sq.file()+1;f<8;++f)
			{
				ret_val[sq][occupancy].set(square(f,sq.rank()));
				if((occupancy>>f)&1)
					break;
			}
			for(int f=sq.file()-1;f>=0;--f)
			{
				ret_val[sq][occupancy].set(square(f,sq.rank()));
				if((occupancy>>f)&1)
					break;
			}
		}
	}
	
	return ret_val;
}

square_table<std::array<bitboard::rank,256>> philchess::detail::compute_bishop_diagonal_attacks() noexcept
{
	square_table<std::array<bitboard::rank,256>> ret_val;
	
	for(std::uint_fast8_t id=0;id<64;++id)
	{
		const auto sq=square{id};
		
		for(std::uint_fast16_t occupancy=0;occupancy<256;++occupancy)
		{
			for(auto file_up=sq.file()+1, rank_up=sq.rank()+1; file_up<8 && rank_up<8;++file_up, ++rank_up)
			{
				ret_val[sq][occupancy].set(square(file_up,rank_up));
				if((occupancy>>file_up)&1)
					break;
			}
			for(auto file_down=sq.file()-1, rank_down=sq.rank()-1; file_down>=0 && rank_down>=0;--file_down, --rank_down)
			{
				ret_val[sq][occupancy].set(square(file_down,rank_down));
				if((occupancy>>file_down)&1)
					break;
			}
		}
	}
	
	return ret_val;
}

square_table<std::array<bitboard::rank,256>> philchess::detail::compute_bishop_antidiagonal_attacks() noexcept
{
	square_table<std::array<bitboard::rank,256>> ret_val;
	
	for(std::uint_fast8_t id=0;id<64;++id)
	{
		const auto sq=square{id};
		
		for(std::uint_fast16_t occupancy=0;occupancy<256;++occupancy)
		{
			for(auto file_up=sq.file()+1, rank_down=sq.rank()-1; file_up<8 && rank_down>=0;++file_up, --rank_down)
			{
				ret_val[sq][occupancy].set(square(file_up,rank_down));
				if((occupancy>>file_up)&1)
					break;
			}
			for(auto file_down=sq.file()-1, rank_up=sq.rank()+1; file_down>=0 && rank_up<8;--file_down, ++rank_up)
			{
				ret_val[sq][occupancy].set(square(file_down,rank_up));
				if((occupancy>>file_down)&1)
					break;
			}
		}
	}
	
	return ret_val;
}

