/*
Copyright (c) 2010 - Wii Banner Player Project

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/

#ifndef WII_BNR_ENDIAN_H_
#define WII_BNR_ENDIAN_H_

#include "CommonFuncs.h"

#include <iostream>

namespace Common
{

// S : size in bytes
template <int S>
inline void SwapData(u8* data);

template <>
inline void SwapData<1>(u8* data)
{
	// nothing
}

template <>
inline void SwapData<2>(u8* data)
{
	*reinterpret_cast<u16*>(data) = swap16(data);
}

template <>
inline void SwapData<4>(u8* data)
{
	*reinterpret_cast<u32*>(data) = swap32(data);
}

template <>
inline void SwapData<8>(u8* data)
{
	*reinterpret_cast<u64*>(data) = swap64(data);
}

}

class BEStream
{
public:
	BEStream(std::istream& stream)
		: m_stream(stream)
	{}

	template <typename V>
	BEStream& operator>>(V& rhs)
	{
		CompileTimeAssert<std::is_arithmetic<V>::value> asrt;
		(void)asrt;

		m_stream.read(reinterpret_cast<char*>(&rhs), sizeof(V));
		Common::SwapData<sizeof(V)>(reinterpret_cast<u8*>(&rhs));

		return *this;
	}

private:
	std::istream& m_stream;
};

struct BEStreamManip
{
	BEStreamManip() {}
};

extern BEStreamManip BE;

inline BEStream operator>>(std::istream& lhs, const BEStreamManip&)
{
	return BEStream(lhs);
}

class LEStream
{
public:
	LEStream(std::istream& stream)
		: m_stream(stream)
	{}

	template <typename V>
	LEStream& operator>>(V& rhs)
	{
		CompileTimeAssert<std::is_arithmetic<V>::value> asrt;
		(void)asrt;

		m_stream.read(reinterpret_cast<char*>(&rhs), sizeof(rhs));

		return *this;
	}

private:
	std::istream& m_stream;
};

struct LEStreamManip
{
	LEStreamManip() {}
};

extern LEStreamManip LE;

inline LEStream operator>>(std::istream& lhs, const LEStreamManip&)
{
	return LEStream(lhs);
}

template <typename T>
void ReadBEArray(std::istream& file, T* data, unsigned int size)
{
	auto& bestrm = file >> BE;

	for (unsigned int i = 0; i != size; ++i)
		bestrm >> data[i];
}

template <typename T>
void ReadLEArray(std::istream& file, T* data, unsigned int size)
{
	auto& lestrm = file >> LE;

	for (unsigned int i = 0; i != size; ++i)
		lestrm >> data[i];
}

#endif
