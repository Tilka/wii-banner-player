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

#ifndef _TYPES_H_
#define _TYPES_H_

#define NOMINMAX
#include <Windows.h>
#include <stdint.h>
#include <string>
#include <iostream>

#include <algorithm>

#include "CommonTypes.h"
#include "CommonFuncs.h"

namespace Common
{

template <int S, typename T>
struct swapX;

template <typename T>
struct swapX<1, T>
{
	static inline T swap(T data)
	{
		return data;
	}
};

template <typename T>
struct swapX<2, T>
{
	static inline T swap(T data)
	{
		const u16 s = swap16((u8*)&data);
		return *(T*)&s;
	}
};

template <typename T>
struct swapX<4, T>
{
	static inline T swap(T data)
	{
		const u32 s = swap32((u8*)&data);
		return *(T*)&s;
	}
};

template <typename T>
struct swapX<8, T>
{
	static inline T swap(T data)
	{
		const u64 s = swap64((u8*)&data);
		return *(T*)&s;
	}
};

template <typename T>
inline T swap(T data)
{
	return swapX<sizeof(T), T>::swap(data);
}

}

struct FourCC
{
	bool operator==(const char str[]) const
	{
		return !memcmp(data, str, sizeof(data));
	}

	bool operator!=(const char str[]) const
	{
		return !(*this == str);
	}

	u8 data[4];
};

inline std::istream& operator>>(std::istream& lhs, FourCC& rhs)
{
	lhs.read((char*)rhs.data, sizeof(rhs.data));

	return lhs;
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
		V value;
		m_stream.read((char*)&value, sizeof(value));
		rhs = Common::swap(value);

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

template <typename C, typename F>
void ForEach(C& container, F func)
{
	std::for_each(container.begin(), container.end(), func);
}

template <typename C, typename F>
void ForEachReverse(C& container, F func)
{
	std::for_each(container.rbegin(), container.rend(), func);
}

template <typename T>
void ReadBEArray(std::istream& file, T* data, unsigned int size)
{
	for (unsigned int i = 0; i != size; ++i)
		file >> BE >> data[i];
}

#endif
