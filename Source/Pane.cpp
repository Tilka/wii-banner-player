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

#include "Pane.h"

#include "WrapGx.h"

// TODO: remove
#include <gl/gl.h>

Pane::Pane(std::istream& file)
	: hide(false)
{
	char pane_name[0x11] = {}; // name in ASCII.
	char user_data[0x09] = {}; // User data	// is it really....?

	file >> BE >> visible >> origin >> alpha;
	file.seekg(1, std::ios::cur);
		
	file.read(pane_name, 0x10);
	file.read(user_data, 0x08);

	file >> BE >> translate.x >> translate.y >> translate.z
		>> rotate.x >> rotate.y >> rotate.z
		>> scale.x >> scale.y
		>> width >> height;

	name = pane_name;
}

Pane::~Pane()
{
	// delete children
	ForEach(panes, [](Pane* const pane)
	{
		delete pane;
	});
}

bool Pane::ProcessRLPA(u8 index, float value)
{
	if (index < 10)
	{
		float* const values[] =
		{
			&translate.x,
			&translate.y,
			&translate.z,

			&rotate.x,
			&rotate.y,
			&rotate.z,

			&scale.x,
			&scale.y,

			&width,
			&height,
		};

		*values[index] = value;
	}
	else
		return false;

	return true;
}

bool Pane::ProcessRLVI(u8 value)
{
	visible = value;

	return true;
}

void Pane::SetFrame(FrameNumber frame)
{
	// setframe on self
	Animator::SetFrame(frame);

	// setframe on children
	ForEach(panes, [&](Pane* pane)
	{
		pane->SetFrame(frame);
	});
}

void Pane::Render() const
{
	if (!visible || hide)
		return;

	glPushMatrix();

	Mtx mt;
	// position
	guMtxTrans(mt, translate.x, translate.y, translate.z);

	// rotate
	guVector vec = { 1.f, 0.f, 0.f };
	guMtxRotAxisRad(mt, &vec, rotate.x);
	vec.x = 0.f; vec.y = 1.f;
	guMtxRotAxisRad(mt, &vec, rotate.y);
	vec.y = 0.f; vec.z = 1.f;
	guMtxRotAxisRad(mt, &vec, rotate.z);

	// scale
	guMtxScale(mt, scale.x, scale.y, 1.f);

	// render self
	Draw();

	// render children
	ForEach(panes, [](const Pane* pane)
	{
		pane->Render();
	});

	glPopMatrix();
}
