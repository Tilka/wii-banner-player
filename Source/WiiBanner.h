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

#include "Animator.h"
#include "Material.h"
#include "Pane.h"
#include "Picture.h"
#include "Texture.h"

#include "Types.h"

class WiiBanner
{
public:
	WiiBanner(const std::string& path);

	void SetFrame(FrameNumber frame);
	void Render();

	void AdvanceFrame();

//private:
	u8 centered; // 1 if layout is drawn from center
	float width, height;

	std::vector<Texture*> textures;
	std::vector<Material*> materials;

	std::vector<Pane*> panes;

	FrameNumber frame_current, frame_loop_start, frame_loop_end;
};

#endif
