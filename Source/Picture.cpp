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

#include <gl/glew.h>

namespace WiiBanner
{

Picture::Picture(std::istream& file, const std::vector<Material*>& materials)
	: Pane(file)
{
	u8 num_texcoords;

	file.read((char*)vertex_colors, 4 * 4);
	file >> BE >> mat_index >> num_texcoords;
	file.seekg(1, std::ios::cur);

	if (mat_index < materials.size())
	{
		material = materials[mat_index];
	}
	else
	{
		material = nullptr;
		std::cout << "Material Index Out Of Range!!!\n";
	}

	// read tex coords
	tex_coords.resize(num_texcoords);
	ReadBEArray(file, &tex_coords[0].coords->s, 2 * 4 * num_texcoords);
}

void Picture::Draw(u8 render_alpha) const
{
	material->Apply();

	glPushMatrix();

	// origin
	glTranslatef(-width / 2 * (origin % 3), -height / 2 * (2 - origin / 3), 0.f);

	// TODO: would adding these offsets to each vertex be faster than push,translate,pop?
	//const float
	//	offx = -width / 2 * (origin % 3),
	//	offy = -height / 2 * (2 - origin / 3);

	// go lambda
	auto const quad_vertex = [=](unsigned int v, float x, float y)
	{
		// color
		glColor4ub(vertex_colors[v][0], vertex_colors[v][1], vertex_colors[v][2],
			MultiplyColors(vertex_colors[v][3], render_alpha));	// apply alpha

		// tex coords
		GLuint i = 0;
		ForEach(tex_coords, [&](const TexCoords& tc)
		{
			glMultiTexCoord2fv(GL_TEXTURE0 + i, &tc.coords[v].s);
			++i;
		});

		// position
		glVertex2f(x, y);
	};

	glBegin(GL_QUADS);
	quad_vertex(2, 0.f, 0.f);
	quad_vertex(3, width, 0.f);
	quad_vertex(1, width, height);
	quad_vertex(0, 0.f, height);
	glEnd();

	glPopMatrix();
}

void Picture::ProcessHermiteKey(const KeyType& type, float value)
{
	if (type.tag == RLVC)	// vertex color
	{
		if (type.target < 0x10)
		{
			// vertex colors
			((u8*)vertex_colors)[type.target] = (u8)value;
			return;
		}
	}
	
	Base::ProcessHermiteKey(type, value);
}

}
