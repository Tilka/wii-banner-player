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

Texture::Texture(std::istream& file)
{
	const std::streamoff file_start = file.tellg();

	// read file header
	FourCC magic; // Magic (0x00, 0x20, 0xAF, 0x30)
	u32 count; // ntextures - Number of Textures in File
	u32 size; // size of Header (always 0x0c in files with this structure)

	file >> magic >> BE >> count >> size;

	//std::cout << "Texture::Texture() count = " << count << '\n';

	// seek to end of header
	file.seekg(size - 0xC, std::ios::cur);

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

		const int size = TexDecoder_GetTextureSizeInBytes(frame.width, frame.height, format);
		const u8* const src = new u8[size];
		file.read((char*)src, size);

		u8* const dst = new u8[frame.width * frame.height * 8];
		TexDecoder_Decode(dst, src, frame.width, frame.height, format, 0, 0, true);
		delete[] src;

		frame.Load(dst);
		delete[] dst;

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

void Texture::Frame::Load(const u8* data)
{
	glGenTextures(1, &gltex);
	glBindTexture(GL_TEXTURE_2D, gltex);

	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
}
