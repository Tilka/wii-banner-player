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

#include <gl/glew.h>

Material::Material(std::istream& file, const std::vector<Texture*>& txtrs)
	: textures(txtrs)
{
	{
	char read_name[21] = {};
	file.read(read_name, 20);
	name = read_name;
	}

	//std::cout << "\tmaterial: " << name << '\n';

	// read colors
	ReadBEArray(file, &color_regs->r, 4 * 3);
	ReadBEArray(file, color_constants[0], sizeof(color_constants));

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

	//std::cout << "flags: " << flags.value << '\n';

	// TextureRef
	for (u32 i = 0; i != flags.texture_ref; ++i)
	{
		TextureRef ref;
		file >> BE >> ref.tex_index >> ref.wrap_s >> ref.wrap_t;

		texture_refs.push_back(ref);
	}

	// TextureSRT
	for (u32 i = 0; i != flags.texture_srt; ++i)
	{
		texture_srts.push_back(TextureSrt());
		auto& srt = texture_srts.back();

		file >> BE >> srt.translate.x >> srt.translate.y >> srt.rotate >> srt.scale.x >> srt.scale.y;

		//std::cout << "XTrans: " << translate.x << " YTrans: " << translate.y << " Rotate: " << rotate
			//<< " XScale: " << scale.x << " YScale: " << scale.y << '\n';
	}
	//if (!flags.texture_srt)
	//{
	//	// set up defaults, this seems dumb/wrong
	//	texture_srts.push_back(TextureSrt());
	//	auto& srt = texture_srts.back();

	//	srt.rotate = 0.f;
	//	srt.scale.x = srt.scale.y = srt.translate.x = srt.translate.y = 1.f;
	//}

	// CoordGen
	for (u32 i = 0; i != flags.texture_coord; ++i)
	{
		texture_coord_gens.push_back(TextureCoordGen());
		auto& coord = texture_coord_gens.back();

		file >> BE >> coord.tgen_type >> coord.tgen_src >> coord.mtrx_src;
		//file >> BE >> tgen_type >> tgen_src >> mtrx_src;
		file.seekg(1, std::ios::cur);
	}
	//if (!flags.texture_coord)
	//{
	//	// set up defaults, this seems dumb/wrong
	//	texture_coord_gens.push_back(TextureCoordGen());
	//	auto& coord = texture_coord_gens.back();

	//	coord.mtrx_src = 30;
	//}

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

		//std::cout << "TevStage\n";
	}
	if (!flags.tev_stage)
	{
		// set up defaults, this seems dumb/wrong

		{
		tev_stages.push_back(TevStage());
		auto& tev = tev_stages.back();

		tev.aC = 2;
		tev.bC = 4;
		tev.cC = 8;
		tev.dC = 0xf;

		tev.aA = 1;
		tev.bA = 2;
		tev.cA = 4;
		tev.dA = 0x7;

		tev.texmap = 0;
		tev.mtxid = 0;
		}

		{
		tev_stages.push_back(TevStage());
		auto& tev = tev_stages.back();

		tev.aC = 0xf;
		tev.bC = 0;
		tev.cC = 10;
		tev.dC = 0xf;

		tev.aA = 0x7;
		tev.bA = 0;
		tev.cA = 5;
		tev.dA = 0x7;

		tev.texmap = 0;
		tev.mtxid = 0;
		}
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

void Material::Apply() const
{
	// alpha compare
	GX_SetAlphaCompare(alpha_compare.function & 0xf, alpha_compare.ref0,
		alpha_compare.aop, alpha_compare.function >> 4, alpha_compare.ref1);

	// blend mode
	GX_SetBlendMode(blend_mode.type, blend_mode.src_factor, blend_mode.dst_factor, blend_mode.logical_op);

	// tev reg colors
	for (unsigned int i = 0; i != 3; ++i)
		GX_SetTevColorS10(GX_TEVREG0 + i, color_regs[i]);

	// bind textures
	{
	unsigned int i = 0;
	ForEach(texture_refs, [&](const TextureRef& tr)
	{
		if (tr.tex_index < textures.size())
		{
			auto& txtr = *textures[tr.tex_index];

			GX_LoadTexObj(&txtr.texobj, i);
			GX_InitTexObjWrapMode(&txtr.texobj, tr.wrap_s, tr.wrap_t);
		}

		++i;
	});
	}

	// texture coord gen
	glMatrixMode(GL_TEXTURE);
	{
	unsigned int i = 0;
	ForEach(texture_coord_gens, [&](const TextureCoordGen& tcg)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glLoadIdentity();

		// TODO: not using "tgen_type", "tgen_src"

		const u8 mtrx = (tcg.mtrx_src - 30) / 3;

		if (mtrx < texture_srts.size())
		{
			const auto& srt = texture_srts[mtrx];

			// why must i do this 0.5 hackery?
			glTranslatef(0.5f, 0.5f, 0.f);
			glRotatef(srt.rotate, 0.f, 0.f, 1.f);
			//glTranslatef(-0.5f, -0.5f, 0.f);

			glTranslatef(-srt.translate.x, -srt.translate.y, 0.f);

			//glTranslatef(0.5f, 0.5f, 0.f);
			glScalef(srt.scale.x, srt.scale.y, 1.f);
			glTranslatef(-0.5f * std::abs(srt.scale.x), -0.5f * std::abs(srt.scale.y), 0.f);
		}

		++i;
	});
	for (; i != 8; ++i)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glLoadIdentity();
	}
	}
	glMatrixMode(GL_MODELVIEW);

	// tev stages
	{
	int i = 0;
	ForEach(tev_stages, [&](const TevStage& ts)
	{
		GX_SetTevOrder(i, ts.texcoord, ts.texmap, ts.color);
		GX_SetTevSwapMode(i, ts.ras_sel, ts.tex_sel);

		GX_SetTevColorIn(i, ts.aC, ts.bC, ts.cC, ts.dC);
		GX_SetTevColorOp(i, ts.tevopC, ts.tevbiasC, ts.tevscaleC, ts.clampC, ts.tevregidC);
		GX_SetTevKColorSel(i, ts.selC);

		GX_SetTevAlphaIn(i, ts.aA, ts.bA, ts.cA, ts.dA);
		GX_SetTevAlphaOp(i, ts.tevopA, ts.tevbiasA, ts.tevscaleA, ts.clampA, ts.tevregidA);
		GX_SetTevKAlphaSel(i, ts.selA);

		GX_SetTevIndirect(i, ts.indtexid, ts.format, ts.bias, ts.mtxid, 
			ts.wrap_s, ts.wrap_t, ts.addprev, ts.utclod, ts.aIND);

		++i;
	});
	
	// enable correct number of tev stages
	GX_SetNumTevStages(i);
	}

	// testing
	//GX_SetNumTevStages(1);

	//glBlendColor((float)color[0] / 255, (float)color[1] / 255, (float)color[2] / 255, (float)color[3] / 255);
}

bool Material::ProcessRLTS(u8 type, u8 index, float value)
{
	if (index < 5 && type < texture_srts.size())
	{
		auto& srt = texture_srts[type];

		float* const values[] =
		{
			&srt.translate.x,
			&srt.translate.y,

			&srt.rotate,

			&srt.scale.x,
			&srt.scale.y,
		};

		*values[index] = value;
	}
	else
		return false;

	return true;
}

bool Material::ProcessRLMC(u8 index, u8 value)
{
	if (index < 4)
	{
		// color
		color[index] = value;
	}
	else if (index < 0x10)
	{
		// initial color of tev color/output registers, often used for foreground/background
		(&color_regs->r)[index - 4] = value;
	}
	else if (index < 0x20)
	{
		// tev color constants
		color_constants[0][index - 0x10] = value;
	}
	else
		return false;

	return true;
}

bool Material::ProcessRLTP(u8 index, u8 value)
{
	// TODO: this aint no good

	//if (index < texture_refs.size())
	//{
	//	texture_refs[index].tex_index = value;
	//}
	//else
		return false;

	return true;
}
