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

#include <gl/gl.h>

namespace WiiBanner
{

Pane::Pane(std::istream& file)
	: hide(false)
{
	file >> BE >> visible >> origin >> alpha;	// TODO: does a pane's alpha affect its children?
	file.seekg(1, std::ios::cur);

	{
	char read_name[0x11] = {};
	file.read(read_name, 0x10);
	name = read_name;
	}

	{
	char user_data[0x09] = {}; // User data	// is it really user data there?
	file.read(user_data, 0x08);
	}

	file >> BE
		>> translate.x >> translate.y >> translate.z
		>> rotate.x >> rotate.y >> rotate.z
		>> scale.x >> scale.y
		>> width >> height;
}

Pane::~Pane()
{
	// delete children
	ForEach(panes, [](Pane* const pane)
	{
		delete pane;
	});
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

	// position
	glTranslatef(translate.x, translate.y, translate.z);

	// rotate
	glRotatef(rotate.x, 1.f, 0.f, 0.f);
	glRotatef(rotate.y, 0.f, 1.f, 0.f);
	glRotatef(rotate.z, 0.f, 0.f, 1.f);

	// scale
	glScalef(scale.x, scale.y, 1.f);

	// render self
	Draw();

	// render children
	ForEach(panes, [](const Pane* pane)
	{
		pane->Render();
	});

	glPopMatrix();
}

void Pane::ProcessHermiteKey(const KeyType& type, float value)
{
	if (type.tag == RLVC)	// vertex color
	{
		// only alpha is supported for Panes afaict
		if (0x10 == type.target)
		{
			alpha = (u8)value;
		}
	}
	else if (type.tag == RLPA)	// pane animation
	{
		if (type.target < 10)
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

			*values[type.target] = value;
		}
	}
	else
		Base::ProcessHermiteKey(type, value);
}

void Pane::ProcessStepKey(const KeyType& type, StepKeyHandler::KeyData data)
{
	if (type.tag == RLVI)	// visibility
	{
		visible = data.data2;
	}
	else
		Base::ProcessStepKey(type, data);
}

}
