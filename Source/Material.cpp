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
{
	char material_name[21] = {};

	union
	{
		u32 hex;

		struct
		{
			u32 texture_ref : 4;
			u32 texture_srt : 4;
			u32 texture_coord : 4;
			u32 tev_swap_mode : 1;
			u32 ind_texture_srt : 2;
			u32 ind_texture_order : 3;
			u32 tev_stage : 5;
			u32 alpha_compare : 1;
			u32 blend_mode : 1;
			u32 channel_control : 1;
			u32 pad1 : 1;
			u32 material_color : 1;
			u32 pad2 : 4;
		};
	} flags;

	file.read(material_name, 20);

	// read colors
	ReadBEArray(file, color_fore, 4);
	ReadBEArray(file, color_back, 4);
	ReadBEArray(file, color_tevreg3, 4);
	ReadBEArray(file, (u8*)color_tevk, sizeof(color_tevk));

	file >> BE >> flags.hex;

	name = material_name;

	std::cout << "\tmaterial: " << name << '\n';

	// TextureRef
	for (u32 i = 0; i != flags.texture_ref; ++i)
	{
		u16 tex_index;
		u8 wrap_s, wrap_t;
		file >> BE >> tex_index >> wrap_s >> wrap_t;

		texture_refs.push_back(TextureRef(tex_index, wrap_s, wrap_t));

		if (tex_index < textures.size())
			texture_refs.back().texture = textures[tex_index];
		else
			std::cout << "Texture Index Out Of Range!!!\n";
	}

	// TextureSRT
	for (u32 i = 0; i != flags.texture_srt; ++i)
	{
		// TODO: make sure not out of range
		TextureRef& ref = texture_refs[i];

		file >> BE >> ref.translate.x >> ref.translate.y >> ref.rotate >> ref.scale.x >> ref.scale.y;

		//std::cout << "XTrans: " << translate.x << " YTrans: " << translate.y << " Rotate: " << rotate
			//<< " XScale: " << scale.x << " YScale: " << scale.y << '\n';
	}
	//if (flags.texture_srt > 1)
	//{
	//	std::cout << "material: " << name << " has " << flags.texture_srt << "texture refs\n";
	//	std::cin.get();
	//}

	// CoordGen
	for (u32 i = 0; i != flags.texture_coord; ++i)
	{
		// TODO: make sure not out of range
		TextureRef& ref = texture_refs[i];

		file >> BE >> ref.tgen_type >> ref.tgen_src >> ref.mtrx_src;
		file.seekg(1, std::ios::cur);
	}

	// ChanControl
	if (flags.channel_control)
	{
		u8 color_matsrc;
		u8 alpha_matsrc;

		file >> BE >> color_matsrc >> alpha_matsrc;
		file.seekg(2, std::ios::cur);

		//if (color_matsrc != alpha_matsrc)
		//{
		//	std::cout << "material: " << name << " has different color and alpha src";
		//	std::cin.get();
		//}

		//std::cout << "color_matsrc: " << (int)color_matsrc << " alpha_matsrc: " << (int)alpha_matsrc << '\n';
	}

	// MaterialColor
	if (flags.material_color)
	{
		u8 color[4];
		ReadBEArray(file, color, 4);

		//std::cout << "color: " << (int)color[0] << ',' << (int)color[1] << ',' << (int)color[2] << '\n';
	}

	// TevSwapModeTable
	if (flags.tev_swap_mode)
	{
		u8 color_tev[4];
		ReadBEArray(file, color_tev, 4);
	}

	// IndTextureSRT
	for (u32 i = 0; i != flags.ind_texture_srt; ++i)
	{
		// TODO: read des guys
		//file >> BE >> translate.x >> translate.y >> rotate >> scale.x >> scale.y;

		file.seekg(5 * 4, std::ios::cur);

		//std::cout << "XTrans: " << translate.x << " YTrans: " << translate.y << " Rotate: " << rotate
			//<< " XScale: " << scale.x << " YScale: " << scale.y << '\n';
	}

	// IndTextureOrder
	for (u32 i = 0; i != flags.ind_texture_order; ++i)
	{
		// TODO: store these
		u8 tex_coord, tex_map, scale_s, scale_t;

		file >> BE >> tex_coord >> tex_map >> scale_s, scale_t;
	}

	// TODO:
	// TevStage
	for (u32 i = 0; i != flags.tev_stage; ++i)
	{

	}

	// TODO:
	// AlphaCompare
	if (flags.alpha_compare)
	{
		//u8 comp, aop, ref0, ref1;
		//file >> BE >> comp >> aop >> ref0 >> ref1;
	}

	// BlendMode
	if (flags.blend_mode)
	{
		//u8 type, src_fact, dst_fact, op;
		//file >> BE >> type >> src_fact >> dst_fact >> op;

		//std::cout << "has blend mode\n";
		//std::cin.get();
	}
}

void Material::Bind() const
{
	if (texture_refs.size())
	{
		const TextureRef& ref = texture_refs.front();
			
		if (ref.texture)
			ref.texture->Bind(0);
		else
			glBindTexture(GL_TEXTURE_2D, 0);

		static const GLenum wraps[] =
		{
			GL_CLAMP,
			GL_REPEAT,
			GL_MIRRORED_REPEAT,
			// TODO: there are more...
		};

		if (ref.wrap_s < 3)
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wraps[ref.wrap_s]);

		if (ref.wrap_t < 3)
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wraps[ref.wrap_t]);
	}
}

void Material::ProcessRLTS(u8 type, u8 index, float value)
{
	if (index < 5 && type < texture_refs.size())
	{
		TextureRef& ref = texture_refs[type];

		float* const values[] =
		{
			&ref.translate.x,
			&ref.translate.y,

			&ref.rotate,

			&ref.scale.x,
			&ref.scale.y,
		};

		*values[index] = value;
	}
}

void Material::AdjustTexCoords(TexCoord tc[]) const
{
	if (texture_refs.empty())
		return;

	// TODO: check if out of range
	const TextureRef& ref = texture_refs.front();

	// TODO: rotate

	// scale
	// TODO: there must be better math?
	auto const expand_values = [](float& v1, float& v2, float scale)
	{
		const float
			avg = (v2 + v1) / 2,
			adj = (v2 - v1) / 2 * scale;

		v2 = avg + adj;
		v1 = avg - adj;
	};

	// scale x
	expand_values(tc[0].s, tc[1].s, ref.scale.x);
	expand_values(tc[2].s, tc[3].s, ref.scale.x);

	// scale y
	expand_values(tc[1].t, tc[2].t, ref.scale.y);
	expand_values(tc[3].t, tc[0].t, ref.scale.y);

	// translate
	for (int i = 0; i != 4; ++i)
	{
		tc[i].s += ref.translate.x;
		tc[i].t += ref.translate.y;
	}
}
