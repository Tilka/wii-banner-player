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

#include "Textbox.h"

namespace WiiBanner
{

Textbox::Textbox(std::istream& file)
	: Pane(file)
{
	file >> BE >> text_buf_bytes >> text_str_bytes
		>> material_index >> font_index >> text_position >> text_alignment;

	file.seekg(2, std::ios::cur);

	file >> BE >> text_str_offset;

	ReadBEArray(file, colors, 2);

	file >> BE >> width >> height >> space_char >> space_line;
}

}
