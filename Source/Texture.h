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

#ifndef WII_BNR_TEXTURE_H_
#define WII_BNR_TEXTURE_H_

#include <list>
#include <string>
#include <vector>

#include "Types.h"

#include "WrapGx.h"

namespace WiiBanner
{

class Texture : public Named
{
public:
	void Load(std::istream& file);

	GXTexObj texobj;
};

class TextureList : public std::vector<Texture*>
{
public:
	static const u32 BINARY_MAGIC = 'txl1';
};

}

#endif
