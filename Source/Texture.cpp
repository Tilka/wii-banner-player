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

#include <fstream>

#include "Texture.h"

static u8 g_texture_read_buffer[512 * 512 * 4];

Texture::Texture(std::istream& file)
{
	const std::streamoff file_start = file.tellg();

	// read file header
	FourCC magic; // Magic (0x00, 0x20, 0xAF, 0x30)
	u32 count; // ntextures - Number of Textures in File
	u32 header_size; // size of Header (always 0x0c in files with this structure)

	file >> magic >> BE >> count >> header_size;

	std::cout << "Texture::Texture() count = " << count << '\n';

	// seek to end of header
	file.seekg(header_size - 0xC, std::ios::cur);

	// only support a single texture
	if (count > 1)
		count = 1;

	// read texture offsets
	std::streamoff next_offset = file.tellg();
	while (count--)
	{
		file.seekg(next_offset, std::ios::beg);

		u32 header_offset;
		u32 palette_offset; // (0 if no palette)

		file >> BE >> header_offset >> palette_offset;

		next_offset = file.tellg();

		if (palette_offset)
			std::cout << "ALERT: palette_offset != 0 is not supported!! " << palette_offset << '\n';

		// seek to the texture header
		file.seekg(file_start + header_offset, std::ios::beg);

		// read texture header
		u32 format;
		u32 offset; // to Texture Data

		u16 height, width;
		u32 wrap_s, wrap_t;
		u32 min_filter, mag_filter;

		float lod_bias;
		u8 edge_lod, min_lod, max_lod;

		u8 unpacked;

		file >> BE >> height >> width >> format >> offset
			>> wrap_s >> wrap_t >> min_filter >> mag_filter
			>> lod_bias >> edge_lod >> min_lod >> max_lod >> unpacked;

		// seek to texture data
		file.seekg(file_start + offset, std::ios::beg);

		// TODO: handle palettes
		// http://pabut.homeip.net:8000/yagcd/chap14.html#sec14.4
		//

		const u32 tex_size = GX_GetTexBufferSize(width, height, format, true, max_lod);

		if (tex_size > sizeof(g_texture_read_buffer))
			std::cout << "texture is too large\n";
		else
		{
			file.read((char*)g_texture_read_buffer, tex_size);

			// load the texture
			GX_InitTexObj(&texobj, g_texture_read_buffer,
				width, height, format, wrap_s, wrap_t, true);

			// filter mode
			GX_InitTexObjFilterMode(&texobj, min_filter, mag_filter);
		}
	}
}

//void Texture::Bind(u32 index) const
//{
//	//if (index < frames.size())
//		//frames[index].Bind();
//}
