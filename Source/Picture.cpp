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

#include "Picture.h"

#include <gl/gl.h>

Picture::Picture(std::istream& file, const std::vector<Material*>& materials)
	: Pane(file)
	, material(NULL)
{
	u8 num_texcoords;

	file.read((char*)vertex_colors, 4 * 4);
	file >> BE >> mat_index >> num_texcoords;
	file.seekg(1, std::ios::cur);

	if (mat_index >= materials.size())
		std::cout << "Material Index Out Of Range!!!\n";
	else
		material = materials[mat_index];

	// read texcoords
	// TODO: only single texcoords set is supported currently
	while (num_texcoords--)
	{
		ReadBEArray(file, (float*)tex_coords, 8);
	}
}

void Picture::Draw() const
{
	material->Bind();

	TexCoord tc[4];
	for (int i = 0; i != 4; ++i)
	{
		tc[i].s = tex_coords[i].s;
		tc[i].t = tex_coords[i].t;
	}

	material->AdjustTexCoords(tc);

	// testing stuff
	u8 vc[4][4];
	memcpy(vc, vertex_colors, 4 * 4);
	
	for (int i = 0; i != 4; ++i)
		vc[i][3] = ((u16)alpha * vc[i][3]) / 255;

	// TODO: vertex_colors/tex_coords corners may be wrong
	// TODO: vertex_colors may need to be byte swapped

	glBegin(GL_POLYGON);

	glColor4ubv(vc[0]);
	glTexCoord2f(tc[0].s, tc[0].t);
	glVertex2f(0.f, height);

	glColor4ubv(vc[1]);
	glTexCoord2f(tc[1].s, tc[1].t);
	glVertex2f(width, height);

	glColor4ubv(vc[3]);
	glTexCoord2f(tc[3].s, tc[3].t);
	glVertex2f(width, 0.f);

	glColor4ubv(vc[2]);
	glTexCoord2f(tc[2].s, tc[2].t);
	glVertex2f(0.f, 0.f);

	glEnd();
}

void Picture::ProcessRLVC(u8 index, float value)
{
	if (index < 16)
		((u8*)vertex_colors)[index] = (u8)value;
	else if (16 == index)
		alpha = (u8)value;
}
