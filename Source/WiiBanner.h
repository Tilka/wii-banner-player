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

#ifndef _WII_BANNER_H_
#define _WII_BANNER_H_

#include <map>
#include <list>
#include <stack>
#include <fstream>

#include "WrapGx.h"

#include "Layout.h"
#include "Picture.h"
#include "Sound.h"

#include "Types.h"

namespace WiiBanner
{

template <typename F>
void ReadOffsetList(std::istream& file, u32 count, std::streamoff origin, F func, std::streamoff pad = 0)
{
	std::streamoff next_offset = file.tellg();

	while (count--)
	{
		file.seekg(next_offset, std::ios::beg);

		u32 offset;
		file >> BE >> offset;
		file.seekg(pad, std::ios::cur);

		next_offset = file.tellg();

		file.seekg(origin + offset, std::ios::beg);
		func();
	}
}

class Banner
{
public:
	Banner(const std::string& filename);
	~Banner();

	Layout* GetBanner() { return layout_banner; }
	Layout* GetIcon() { return layout_icon; }

	BannerStream sound;

private:
	Layout *layout_banner, *layout_icon;
};

}

#endif
