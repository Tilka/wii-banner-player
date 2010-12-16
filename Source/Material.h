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

#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include "Types.h"

#include "Animator.h"
#include "Texture.h"

struct TexCoord
{
	float s, t;
};

class Material : public Animator
{
public:
	Material(std::istream& file, const std::vector<Texture*>& textures);

	void Bind() const;

	void ProcessRLTS(u8 type, u8 index, float value);
	void ProcessRLMC(u8 index, u8 value);

	void AdjustTexCoords(TexCoord tc[]) const;
	
	const s16* GetColorFore() const { return color_fore; }
	const s16* GetColorBack() const { return color_back; }

	~Material()
	{
		//glDeleteTextures(1, &gltex);
	}

//protected:

	struct TextureRef
	{
		TextureRef(u16 _tex_index, u8 _wrap_s, u8 _wrap_t)
			: tex_index(_tex_index)
			, wrap_s(_wrap_s), wrap_t(_wrap_t)
			, texture(NULL)
		{
			translate.x = translate.y = scale.x = scale.y = 1.f;
			rotate = 0;
		}

		struct
		{
			float x, y;
		} translate, scale;

		float rotate;

		u16 tex_index;
		u8 wrap_s, wrap_t;

		u8 tgen_type, tgen_src, mtrx_src;	// TODO: initialize these guys

		Texture* texture;
	};

	std::vector<TextureRef> texture_refs;

	struct
	{
		u8 type, src_factor, dst_factor, op;

	} blend_mode;

	struct
	{
		u8 function, aop, ref0, ref1;

	} alpha_compare;

	union
	{
		u8 value;

		struct
		{
			u8 red : 2;
			u8 green : 2;
			u8 blue : 2;
			u8 alpha : 2;
		};

	} tev_swap_mode_table[4];

	u8 color[4];

	// why are these 2 byte values?
	s16 color_fore[4];
	s16 color_back[4];
	s16 color_tevreg3[4];	// wtf is a reg3
	u8 color_tevk[4][4];	// wtf is s tevk1,tevk2 etc...

	// TODO: probably temporary
	u8 palette_index;
};

#endif
