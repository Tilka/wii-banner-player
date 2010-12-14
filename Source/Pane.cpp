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

Pane::Pane(std::istream& file)
{
	char pane_name[0x11] = {}; // name in ASCII.
	char user_data[0x09] = {}; // User data

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

void Pane::ProcessRLPA(u8 index, float value)
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

	if (index < 10)
		*values[index] = value;
}

void Pane::ProcessRLVI(u8 value)
{
	visible = value;
}

void PaneHolder::SetFrame(FrameNumber frame)
{
	Animator::SetFrame(frame);

	ForEach(panes, [&](Pane* pane)
	{
		pane->SetFrame(frame);
	});
}

void Pane::Render() const
{
	if (!visible)
		return;

	glPushMatrix();

	// testing
	//glPixelTransferf(GL_ALPHA_SCALE, 0.5f);
	//glPixelTransferf(GL_BLUE_SCALE, 0.0f);

	// position
	glTranslatef(translate.x, translate.y, translate.z);

	// scale
	glScalef(scale.x, scale.y, 1.f);

	// origin
	glTranslatef(-width / 2 * (origin % 3), -height / 2 * (origin / 3), 0);

	// rotations
	glRotatef(rotate.x, 1.f, 0.f, 0.f);
	glRotatef(rotate.y, 0.f, 1.f, 0.f);
	glRotatef(rotate.z, 0.f, 0.f, 1.f);

	Draw();

	glPopMatrix();
}

void PaneHolder::Draw() const
{
	// undo origin
	glTranslatef(width / 2 * (origin % 3), height / 2 * (origin / 3), 0);

	ForEach(panes, [](const Pane* pane)
	{
		pane->Render();
	});
}
