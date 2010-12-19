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

#include "WrapGx.h"

//#include <gl/glew.h>

// TODO: put SAFE_ARRAY_VALUE stuff elsewhere
#define ARRAY_LENGTH(a) (sizeof(a)/sizeof(*a))
#define SAFE_ARRAY_VALUE(a, x) (((x) < ARRAY_LENGTH(a)) ? (a[x]) : (a[0]))

Material::Material(std::istream& file, const std::vector<Texture*>& textures)
{
	{
	char read_name[21] = {};
	file.read(read_name, 20);
	name = read_name;
	}

	std::cout << "\tmaterial: " << name << '\n';

	// read colors
	ReadBEArray(file, color_fore, 4);
	ReadBEArray(file, color_back, 4);
	ReadBEArray(file, color_tevreg3, 4);
	ReadBEArray(file, (u8*)color_tevk, sizeof(color_tevk));

	union
	{
		u32 value;

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

	file >> BE >> flags.value;

	std::cout << "flags: " << flags.value << '\n';

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
	//	std::cout << flags.texture_srt << "texture refs\n";
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
		u8 color_matsrc, alpha_matsrc;
		//u8 pad1, pad2;

		file >> BE >> color_matsrc >> alpha_matsrc;// >> pad1 >> pad2;
		file.seekg(2, std::ios::cur);

		if (color_matsrc != alpha_matsrc)
		{
			//std::cout << "color: " << (int)color_matsrc
			//	<< " alpha: " << (int)alpha_matsrc
			//	//<< " pad1: " << (int)pad1
			//	//<< " pad2: " << (int)pad2
			//	<< '\n';
			//std::cin.get();
		}

		//std::cout << "color_matsrc: " << (int)color_matsrc << " alpha_matsrc: " << (int)alpha_matsrc << '\n';
	}

	// MaterialColor
	if (flags.material_color)
	{
		ReadBEArray(file, color, 4);

		// these are like always 255
		//std::cout << "color: " << (int)color[0] << ',' << (int)color[1] << ',' << (int)color[2] << '\n';
		//std::cin.get();
	}
	else
	{
		memset(color, 255, 4);
	}

	// TevSwapModeTable
	if (flags.tev_swap_mode)
	{
		ReadBEArray(file, tev_swap_mode_table, 4);

		//std::cout << "TevSwapModeTable:\n";
		//for (unsigned int i = 0; i != 4; ++i)
		//	std::cout << "\t[" << i << "]: red: " << (int)tev_swap_mode_table[i].red
		//		<< " green: " << (int)tev_swap_mode_table[i].green
		//		<< " blue: " << (int)tev_swap_mode_table[i].blue
		//		<< " alpha: " << (int)tev_swap_mode_table[i].alpha << '\n';
	}

	// IndTextureSRT
	for (u32 i = 0; i != flags.ind_texture_srt; ++i)
	{
		// TODO: read des guys
		//file >> BE >> translate.x >> translate.y >> rotate >> scale.x >> scale.y;

		//file.seekg(5 * 4, std::ios::cur);

		//std::cout << "ind_texture: SRT\n";
	}

	// IndTextureOrder
	for (u32 i = 0; i != flags.ind_texture_order; ++i)
	{
		// TODO: store these
		u8 tex_coord, tex_map, scale_s, scale_t;

		file >> BE >> tex_coord >> tex_map >> scale_s, scale_t;

		file.ignore(1);

		//std::cout << "ind_texture: " << name << "tex_coord: " << (int)tex_coord
		//	<< " tex_map: " << (int)tex_map << '\n';
		//std::cin.get();
	}

	// TevStage
	for (u32 i = 0; i != flags.tev_stage; ++i)
	{
		tev_stages.push_back(TevStage());

		file.read((char*)&tev_stages.back(), sizeof(TevStage));

		std::cout << "TevStage\n";
	}

	// TODO:
	// AlphaCompare
	if (flags.alpha_compare)
	{
		file >> BE >> alpha_compare.function >> alpha_compare.aop
			>> alpha_compare.ref0 >> alpha_compare.ref1;

		//std::cout << "alpha compare:\t"
		//	<< " function: " << (int)alpha_compare.function
		//	<< " aop: " << (int)alpha_compare.aop << " ref0: " << (int)alpha_compare.ref0
		//	<< " ref1: " << (int)alpha_compare.ref1 << '\n';
		//std::cin.get();
	}
	else
	{
		alpha_compare.function = 0x66;
		alpha_compare.ref0 = 0;
		alpha_compare.ref1 = 0;
		//alpha_compare.aop = ;
	}

	// BlendMode
	if (flags.blend_mode)
	{
		file >> BE >> blend_mode.type >> blend_mode.src_factor >> blend_mode.dst_factor >> blend_mode.logical_op;

		//std::cout << "blend mode:\t"
		//	<< " type: " << (int)blend_mode.type
		//	<< " src: " << (int)blend_mode.src_factor << " dst: " << (int)blend_mode.dst_factor
		//	<< " op: " << (int)blend_mode.logical_op << '\n';
		//std::cin.get();
	}
	else
	{
		blend_mode.type = 1;
		blend_mode.src_factor = 4;
		blend_mode.dst_factor = 5;
		blend_mode.logical_op = 3;
	}
}

void Material::Bind() const
{
	// bind textures
	{
	//glMatrixMode(GL_TEXTURE);

	unsigned int i = 0;
	ForEach(texture_refs, [&](const TextureRef& tr)
	{
		if (tr.texture)
		{
			GX_LoadTexObj(&tr.texture->texobj, i);
			GX_InitTexObjWrapMode(&tr.texture->texobj, tr.wrap_s, tr.wrap_t);

			//// temporary
			//glLoadIdentity();
			//glTranslatef(tr.translate.x, tr.translate.y, 0.f);
			//glRotatef(tr.rotate, 0.f, 0.f, 1.f);
			//glScalef(tr.scale.x, tr.scale.y, 0.f);
		}

		++i;
	});

	//glMatrixMode(GL_MODELVIEW);
	}

	// alpha compare
	GX_SetAlphaCompare(alpha_compare.function & 0xf, alpha_compare.ref0,
		alpha_compare.aop, alpha_compare.function >> 4, alpha_compare.ref1);

	// blend mode
	GX_SetBlendMode(blend_mode.type, blend_mode.src_factor, blend_mode.dst_factor, blend_mode.logical_op);

	// tev stages
	{
	int i = 0;
	ForEach(tev_stages, [&](const TevStage& ts)
	{
		GX_SetTevOrder(i, ts.texcoord, ((u32)ts.texmaptop << 8) | ts.texmapbot, ts.color);
		GX_SetTevSwapMode(i, ts.ras_sel, ts.tex_sel);

		GX_SetTevIndirect(i, ts.indtexid, ts.format, ts.bias, ts.mtxid, 
			ts.wrap_s, ts.wrap_t, ts.addprev, ts.utclod, ts.aIND);

		GX_SetTevColorIn(i, ts.aC, ts.bC, ts.cC, ts.dC);
		GX_SetTevColorOp(i, ts.tevopC, ts.tevbiasC, ts.tevscaleC, ts.clampC, ts.tevregidC);
		GX_SetTevKColorSel(i, ts.selC);

		GX_SetTevAlphaIn(i, ts.aA, ts.bA, ts.cA, ts.dA);
		GX_SetTevAlphaOp(i, ts.tevopA, ts.tevbiasA, ts.tevscaleA, ts.clampA, ts.tevregidA);
		GX_SetTevKAlphaSel(i, ts.selA);

		++i;
	});

	// no tev stages defined, set up defaults
	if (0 == i)
	{
		// one stage each texture reference, i guess?
		ForEach(texture_refs, [&](const TextureRef& tr)
		{
			GX_SetTevOrder(i, 0, i, 0);
			//GX_SetTevSwapMode(0, ts.ras_sel, ts.tex_sel);

			//GX_SetTevIndirect(0, ts.indtexid, ts.format, ts.bias, ts.mtxid, 
				//ts.wrap_s, ts.wrap_t, ts.addprev, ts.utclod, ts.aIND);

			GX_SetTevColorIn(i, 0xf, 8, 10, 0xf);
			GX_SetTevColorOp(i, 0, 0, 0, 0, 0);
			//GX_SetTevKColorSel(0, ts.selC);

			GX_SetTevAlphaIn(i, 0x7, 4, 5, 0x7);
			GX_SetTevAlphaOp(i, 0, 0, 0, 0, 0);
			//GX_SetTevKAlphaSel(0, ts.selA);

			++i;
		});

		// no texture references, set up a tev stage without texture
		if (0 == i)
		{
			GX_SetTevOrder(i, 0, 0, 0);

			GX_SetTevColorIn(i, 0xf, 0xf, 0xf, 10);
			GX_SetTevColorOp(i, 0, 0, 0, 0, 0);

			GX_SetTevAlphaIn(i, 0x7, 0x7, 0x7, 5);
			GX_SetTevAlphaOp(i, 0, 0, 0, 0, 0);

			++i;
		}
	}
	
	// enable correct number of tev stages
	GX_SetNumTevStages(i);
	}

	// testing
	//GX_SetNumTevStages(1);

	//glBlendColor((float)color[0] / 255, (float)color[1] / 255, (float)color[2] / 255, (float)color[3] / 255);
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

void Material::ProcessRLMC(u8 index, u8 value)
{
	if (index < 4)
		color[index] = value;
	else if (index < 8)
		color_fore[index - 4] = value;
	else if (index < 12)
		color_back[index - 8] = value;
	else if (index < 16)
		color_tevreg3[index - 12] = value;
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
