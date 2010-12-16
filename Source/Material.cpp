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
				//<< " alpha: " << (int)alpha_matsrc
				//<< " pad1: " << (int)pad1
				//<< " pad2: " << (int)pad2
				//<< '\n';
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

	// TODO:
	// TevStage
	for (u32 i = 0; i != flags.tev_stage; ++i)
	{
		tev_stages.push_back(TevStages());

		file.read((char*)&tev_stages.back(), sizeof(TevStages));

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

		std::cout << "blend mode:\t"
			<< " type: " << (int)blend_mode.type
			<< " src: " << (int)blend_mode.src_factor << " dst: " << (int)blend_mode.dst_factor
			<< " op: " << (int)blend_mode.logical_op << '\n';
		std::cin.get();
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
	if (texture_refs.size())
	{
		const TextureRef& ref = texture_refs.front();
			
		if (ref.texture)
			ref.texture->Bind(0);

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
	else
		glBindTexture(GL_TEXTURE_2D, 0);

	if ((alpha_compare.function & 0xf) < 7)
	{
		static const GLenum alpha_funcs[] =
		{
			GL_NEVER,
			GL_EQUAL,
			GL_LEQUAL,
			GL_GREATER,
			GL_NOTEQUAL,
			GL_GEQUAL,
			GL_ALWAYS,
		};

		glAlphaFunc(alpha_funcs[alpha_compare.function & 0xf], (float)alpha_compare.ref0 / 255.f);
	}

	if (blend_mode.type < 4)
	{
		static const GLenum blend_modes[] =
		{
			GL_MAX,	// none?
			GL_FUNC_ADD,	// BLEND??
			GL_FUNC_REVERSE_SUBTRACT,	// LOGIC??
			GL_FUNC_SUBTRACT,
		};

		//if (blend_mode.type)
		{
			glEnable(GL_BLEND);
			glBlendEquation(blend_modes[blend_mode.type]);
		}
		//else
		{
			//glDisable(GL_BLEND);
		}
	}

	if (blend_mode.src_factor < 8 && blend_mode.dst_factor < 8)
	{
		static const GLenum blend_factors[] =
		{
			GL_ZERO,
			GL_ONE,
			GL_SRC_COLOR,
			GL_ONE_MINUS_SRC_COLOR,
			GL_SRC_ALPHA,
			GL_ONE_MINUS_SRC_ALPHA,
			GL_DST_ALPHA,
			GL_ONE_MINUS_DST_ALPHA,
		};

		glBlendFunc(blend_factors[blend_mode.src_factor], blend_factors[blend_mode.dst_factor]);
	}

	if (blend_mode.logical_op < 16)
	{
		static const GLenum logic_ops[] =
		{
			GL_CLEAR,
			GL_AND,
			GL_AND_REVERSE,
			GL_COPY,
			GL_AND_INVERTED,
			GL_NOOP,
			GL_XOR,
			GL_OR,
			GL_NOR,
			GL_EQUIV,
			GL_INVERT,
			GL_OR_REVERSE,
			GL_COPY_INVERTED,
			GL_OR_INVERTED,
			GL_NAND,
			GL_SET,
		};

		glLogicOp(logic_ops[blend_mode.logical_op]);
	}

	unsigned int i = 0;
	ForEach(tev_stages, [&](const TevStages& ts)
	{
		static const GLenum txts[] =
		{
			GL_TEXTURE0,
			GL_TEXTURE1,
			GL_TEXTURE2,
			GL_TEXTURE3,
			GL_TEXTURE4,
		};

		glActiveTexture(txts[i]);

		glTexEnvi(GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1 << ts.tevscaleA);
		glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE, 1 << ts.tevscaleC);

		++i;
	});

	glActiveTexture(GL_TEXTURE0);

	// not good?
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
