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

#include "TextureDecoder.h"

static u8 g_texture_read_buffer[512 * 512 * 4];
static u8 g_texture_decode_buffer[512 * 512 * 4];

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

		Frame frame;

		file >> BE >> frame.height >> frame.width >> format >> offset
			>> frame.wrap_s >> frame.wrap_t >> frame.min_filter >> frame.mag_filter
			>> frame.lod_bias >> frame.edge_lod >> frame.min_lod >> frame.max_lod >> frame.unpacked;

		// seek to texture data
		file.seekg(file_start + offset, std::ios::beg);

		// TODO: handle palettes
		// http://pabut.homeip.net:8000/yagcd/chap14.html#sec14.4
		//

		const u32 bsw = TexDecoder_GetBlockWidthInTexels(format) - 1;
		const u32 bsh = TexDecoder_GetBlockHeightInTexels(format) - 1;

		const u32 expanded_width  = (frame.width  + bsw) & (~bsw);
		const u32 expanded_height = (frame.height + bsh) & (~bsh);

		const int tex_size = TexDecoder_GetTextureSizeInBytes(expanded_width, expanded_height, format);

		GLenum gl_format, gl_iformat, gl_type = 0;

		if (tex_size > sizeof(g_texture_read_buffer) || (frame.width * frame.height * 4) > sizeof(g_texture_decode_buffer))
			std::cout << "texture is too large\n";
		else
		{
			file.read((char*)g_texture_read_buffer, tex_size);

			auto const pcfmt = TexDecoder_Decode(g_texture_decode_buffer,
				g_texture_read_buffer, expanded_width, expanded_height, format, 0, 0);

			switch (pcfmt)
			{
			default:
			case PC_TEX_FMT_NONE:
				std::cout << "Error decoding texture!!!\n";

			case PC_TEX_FMT_BGRA32:
				gl_format = GL_BGRA;
				gl_iformat = 4;
				gl_type = GL_UNSIGNED_BYTE;
				break;

			case PC_TEX_FMT_RGBA32:
				gl_format = GL_RGBA;
				gl_iformat = 4;
				gl_type = GL_UNSIGNED_BYTE;
				break;

			case PC_TEX_FMT_I4_AS_I8:
				gl_format = GL_LUMINANCE;
				gl_iformat = GL_INTENSITY4;
				gl_type = GL_UNSIGNED_BYTE;
				break;

			case PC_TEX_FMT_IA4_AS_IA8:
				gl_format = GL_LUMINANCE_ALPHA;
				gl_iformat = GL_LUMINANCE4_ALPHA4;
				gl_type = GL_UNSIGNED_BYTE;
				break;

			case PC_TEX_FMT_I8:
				gl_format = GL_LUMINANCE;
				gl_iformat = GL_INTENSITY8;
				gl_type = GL_UNSIGNED_BYTE;
				break;

			case PC_TEX_FMT_IA8:
				gl_format = GL_LUMINANCE_ALPHA;
				gl_iformat = GL_LUMINANCE8_ALPHA8;
				gl_type = GL_UNSIGNED_BYTE;
				break;

			case PC_TEX_FMT_RGB565:
				gl_format = GL_RGB;
				gl_iformat = GL_RGB;
				gl_type = GL_UNSIGNED_SHORT_5_6_5;
				break;
			}
		}

		frame.Load(g_texture_decode_buffer, expanded_width, gl_format, gl_iformat, gl_type);

		frames.push_back(frame);
	}
}

void Texture::Bind(u32 index) const
{
	if (index < frames.size())
		frames[index].Bind();
}

void Texture::Frame::Bind() const
{
	glBindTexture(GL_TEXTURE_2D, gltex);

	// TODO: set texture params
}

void Texture::Frame::Load(const u8* data, u32 expanded_width, GLenum format, GLenum iformat, GLenum type)
{
	glGenTextures(1, &gltex);
	glBindTexture(GL_TEXTURE_2D, gltex);

	if (expanded_width != width)
        glPixelStorei(GL_UNPACK_ROW_LENGTH, expanded_width);

	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexImage2D(GL_TEXTURE_2D, 0, iformat, width, height, 0, format, type, data);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);

	if (expanded_width != width)
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
}
