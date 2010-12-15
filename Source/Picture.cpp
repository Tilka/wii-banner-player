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

// assumes u8s, takes any type to avoid multiple conversions
template <typename C1, typename C2>
inline u8 MultiplyColors(C1 c1, C2 c2)
{
	return (u16)c1 * c2 / 0xff;
}

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

	const s16* const mat_back = material->GetColorBack();

	//std::cout << "mat_back[3]: " << mat_back[3] << '\n';
	
	for (int vert = 0; vert != 4; ++vert)
	{
		vc[vert][3] = MultiplyColors(vc[vert][3], alpha);
		
		for (int c = 0; c != 4; ++c)
			vc[vert][c] = MultiplyColors(vc[vert][c], mat_back[c]);
	}

	glBegin(GL_POLYGON);

	glColor4ubv(vc[2]);
	glTexCoord2f(tc[2].s, tc[2].t);
	glVertex2f(0.f, 0.f);

	glColor4ubv(vc[3]);
	glTexCoord2f(tc[3].s, tc[3].t);
	glVertex2f(width, 0.f);

	glColor4ubv(vc[1]);
	glTexCoord2f(tc[1].s, tc[1].t);
	glVertex2f(width, height);

	glColor4ubv(vc[0]);
	glTexCoord2f(tc[0].s, tc[0].t);
	glVertex2f(0.f, height);

	glEnd();
}

void Picture::ProcessRLVC(u8 index, u8 value)
{
	if (index < 16)
		((u8*)vertex_colors)[index] = value;
	else if (16 == index)
		alpha = value;
}
