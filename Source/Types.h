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

#ifndef WII_BNR_TYPES_H_
#define WII_BNR_TYPES_H_

#define NOMINMAX
#include <Windows.h>

#include <stdint.h>
#include <string>
#include <iostream>

#include <algorithm>
#include <type_traits>

#include "CommonTypes.h"
#include "CommonFuncs.h"

// TODO: remove
#include "Endian.h"

struct Vec2
{
	float x, y;
};

struct Vec3
{
	float x, y, z;
};

class FourCC
{
	friend std::istream& operator>>(std::istream& lhs, FourCC& rhs);
	friend std::ostream& operator<<(std::ostream& lhs, const FourCC& rhs);

public:
	FourCC() {}
	FourCC(u32 _data) { data = _data; }

	bool operator==(u32 rhs) const
	{
		return data == rhs;
	}

	bool operator!=(u32 rhs) const
	{
		return !(*this == rhs);
	}

private:
	u32 data;
};

#define MAKE_FOURCC(a, b, c, d) ((a) * (1 << 24) + (b) * (1 << 16) + (c) * (1 << 8) + (d) * (1 << 0))

inline std::istream& operator>>(std::istream& lhs, FourCC& rhs)
{
	lhs >> BE >> rhs.data;
	return lhs;
}

inline std::ostream& operator<<(std::ostream& lhs, const FourCC& rhs)
{
	const u32 data = Common::swap32(rhs.data);
	lhs.write(reinterpret_cast<const char*>(&data), 4);
	return lhs;
}

template <typename C, typename F>
inline void ForEach(C& container, F func)
{
	std::for_each(container.begin(), container.end(), func);
}

//template <typename C, typename F>
//void ForEachReverse(C& container, F func)
//{
//	std::for_each(container.rbegin(), container.rend(), func);
//}

template <typename T>
inline T Clamp(T value, T min, T max)
{
	return (value < min) ? min : (value < max) ? value : max;
}

template <typename T, typename B>
inline T RoundUp(T value, B base)
{
	return static_cast<T>((value + (base - 1)) & ~(base - 1));
}

template <typename T, typename B>
inline T RoundDown(T value, B base)
{
	return static_cast<T>(value & ~(base - 1));
}

template <typename C, typename F>
void ReadSections(std::istream& file, C count, F func)
{	
	while (count--)
	{
		const std::streamoff start = file.tellg();

		FourCC magic;
		u32 size = 0;
		file >> magic >> BE >> size;

		func(magic, start);

		file.seekg(start + size, std::ios::beg);
	}
}

class Named
{
public:
	std::string& GetName() { return name; }	// how silly is this?
	const std::string& GetName() const { return name; }
	void SetName(const std::string& _name) { name = _name; }

private:
	std::string name;
};

inline std::string ReadNullTerminatedString(std::istream& file)
{
	std::string str;
	std::getline(file, str, '\0');
	return str;
}

template <int L>
std::string ReadFixedLengthString(std::istream& file)
{
	char str[L + 1] = {};
	file.read(str, L);
	return str;
}

inline void WriteNullTerminatedString(std::ostream& file, const std::string& str)
{
	file << str << '\0';
}

template <int L>
inline void WriteFixedLengthString(std::ostream& file, const std::string& str)
{
	char str[L] = {};
	std::copy(str.begin(), (str.length() > L) ? str.begin() + L : str.end(), str);
	file.write(str, L);
}

#endif
