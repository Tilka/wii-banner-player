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

#include "Material.h"

// TODO: put SAFE_ARRAY_VALUE stuff elsewhere
#define ARRAY_LENGTH(a) (sizeof(a)/sizeof(*a))
#define SAFE_ARRAY_VALUE(a, x) (((x) < ARRAY_LENGTH(a)) ? (a[x]) : (a[0]))

Material::Material(std::istream& file, const std::vector<Texture*>& textures)
	: texture(NULL)
{
	memset(color, 255, 4);

	wrap_s = 0;
	wrap_t = 0;

	char material_name[21] = {};
	s16 fore_color[4];
	s16 back_color[4];
	s16 tevREG3_color[4];
	u32 tev_kcolor[4];

	union
	{
		u32 hex;

		// TODO: name these better
		struct
		{
			u32 texture : 4;
			u32 tex_srt : 4;
			u32 tex_coord : 4;
			u32 ua6 : 1;
			u32 ua7 : 2;
			u32 ua8 : 3;
			u32 ua9 : 5;
			u32 uaa : 1;
			u32 uab : 1;
			u32 ua4 : 1;
			u32 padding1 : 1;
			u32 ua5 : 1;
			u32 padding2 : 4;
		};
	} flags;

	file.read(material_name, 20);
	ReadBEArray(file, fore_color, 4);
	ReadBEArray(file, back_color, 4);
	ReadBEArray(file, tevREG3_color, 4);
	ReadBEArray(file, tev_kcolor, 4);
	file >> BE >> flags.hex;

	name = material_name;

	std::cout << "\tmaterial: " << name << '\n';

	// texture reference
	// TODO: only single reference supported
	for (u32 i = 0; i != flags.texture; ++i)
	{
		file >> BE >> tex_index >> wrap_s >> wrap_t;

		if (tex_index >= textures.size())
			std::cout << "Texture Index Out Of Range!!!\n";
		else
			texture = textures[tex_index];
	}

	scale.x = 1;
	scale.y = 1;

	// srt
	for (u32 i = 0; i != flags.tex_srt; ++i)
	{
		file >> BE >> translate.x >> translate.y >> rotate >> scale.x >> scale.y;

		std::cout << "XTrans: " << translate.x << " YTrans: " << translate.y << " Rotate: " << rotate
			<< " XScale: " << scale.x << " YScale: " << scale.y << '\n';
	}

	// coord
	for (u32 i = 0; i != flags.tex_coord; ++i)
	{
		char tgen_type;
		char tgen_src;
		char mtxsrc;

		file >> BE >> tgen_type >> tgen_src >> mtxsrc;
		file.seekg(1, std::ios::cur);
	}

	// color/alpha texture source
	if (flags.ua4)
	{
		u8 color_matsrc;
		u8 alpha_matsrc;

		file >> BE >> color_matsrc >> alpha_matsrc;
		file.seekg(2, std::ios::cur);

		//std::cout << "color_matsrc: " << (int)color_matsrc << " alpha_matsrc: " << (int)alpha_matsrc << '\n';
	}

	// color
	if (flags.ua5)
	{
		ReadBEArray(file, color, 4);

		//std::cout << "color: " << (int)color[0] << ',' << (int)color[1] << ',' << (int)color[2] << '\n';
	}
}

GLenum wraps[] =
{
	GL_CLAMP,
	GL_REPEAT,
	GL_MIRRORED_REPEAT,
	// TODO: there are more
};

void Material::Bind() const
{
	if (texture)
		texture->Bind(0);
	//else
		//std::cout << "Material has no texture: " << name << '\n';

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, SAFE_ARRAY_VALUE(wraps, wrap_s));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, SAFE_ARRAY_VALUE(wraps, wrap_t));
}

void Material::ProcessRLTS(u8 index, float value)
{
	float* const values[] =
	{
		&translate.x,
		&translate.y,

		&rotate,

		&scale.x,
		&scale.y,
	};

	if (index < 5)
		*values[index] = value;
}

void Material::AdjustTexCoords(TexCoord tc[]) const
{
	for (int i = 0; i != 4; ++i)
	{
		tc[i].s = (tc[i].s + translate.x) * scale.x;
		tc[i].t = (tc[i].t + translate.y) * scale.y;
	}

	//glScalef(1 / scale.x, 1 / scale.y, 1.f);
}
