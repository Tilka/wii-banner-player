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

#include "Layout.h"
#include "Picture.h"

#include <stack>

#include <gl/glew.h>

namespace WiiBanner
{

struct SectionHeader
{
	SectionHeader(std::istream& file)
		: start(file.tellg()), size(0)
	{}

	FourCC magic;
	u32 size;

	std::streamoff start;
};

// read a section header
inline std::istream& operator>>(std::istream& lhs, SectionHeader& rhs)
{
	rhs.start = lhs.tellg();
	lhs >> rhs.magic >> BE >> rhs.size;

	return lhs;
}

// seek past a section
inline std::istream& operator+=(std::istream& lhs, const SectionHeader& rhs)
{
	lhs.seekg(rhs.start + rhs.size, std::ios::beg);

	return lhs;
}

template <typename F>
void ReadOffsetList(std::istream& file, u32 count, std::streamoff origin, F func, std::streamoff pad = 0)
{
	std::streamoff next_offset = file.tellg();

	while (count--)
	{
		file.seekg(next_offset, std::ios::beg);

		u32 offset;
		file >> BE >> offset;
		file.seekg(pad, std::ios::cur);

		next_offset = file.tellg();

		file.seekg(origin + offset, std::ios::beg);
		func();
	}
}

Layout::Layout(std::istream& file)
	: frame_current(0)
	, frame_loop_start(0)
	, frame_loop_end(0)
	, width(0), height(0)
	, centered(0)
{
	const std::streamoff file_start = file.tellg();

	// read header
	FourCC magic; // RLYT
	u16 endian; // 0xFEFF
	u16 version; // 0x0008
	u32 filesize;
	u16 offset; // offset to first section
	u16 section_count;

	file >> magic >> BE >> endian >> version
		>> filesize >> offset >> section_count;

	if (magic != "RLYT"
		|| endian != 0xFEFF
		|| version != 0x008
		)
		return;	// bad header

	// temporary maps and stacks
	std::map<std::string, Pane*> pane_animator_map;
	std::map<std::string, Material*> mate_animator_map;

	Group* last_group = NULL;
	std::stack<std::map<std::string, Group>*> group_stack;
	group_stack.push(&groups);

	Pane* last_pane = NULL;
	std::stack<std::vector<Pane*>*> pane_stack;
	pane_stack.push(&panes);

	// seek to the first section
	file.seekg(file_start + offset, std::ios::beg);

	// read each section
	SectionHeader header(file);
	while (section_count--)
	{
		file += header;
		file >> header;

		if (header.magic == "lyt1")
		{
			// read layout
			file >> BE >> centered;
			file.seekg(3, std::ios::cur);
			file >> BE >> width >> height;
		}
		else if (header.magic == "txl1")
		{
			// load textures
			u16 texture_count;
			u16 offset;

			file >> BE >> texture_count >> offset;

			ReadOffsetList(file, texture_count, file.tellg(), [&]
			{
				std::string texture_name;
				std::getline(file, texture_name, '\0');

				auto* const texture = new Texture(texture_name);

				//file.seekg(archive.GetFileOffset("arc/timg/" + texture_name), std::ios::beg);
				//texture->Load(file);

				textures.push_back(texture);

			}, 4);

			std::cout << "Loaded " << textures.size() << " Textures\n";
		}
		else if (header.magic == "mat1")
		{
			// load materials
			u16 material_count; // num materials
			u16 offset; // Offset to list start. Always zero

			file >> BE >> material_count >> offset;

			ReadOffsetList(file, material_count, header.start, [&]
			{
				Material* const mate = new Material(file, textures);
				materials.push_back(mate);
				mate_animator_map[materials.back()->name] = mate;
			});

			std::cout << "Loaded " << materials.size() << " Materials\n";
		}
		else if (header.magic == "pic1")
		{
			Picture* const pic = new Picture(file, materials);
			pane_stack.top()->push_back(last_pane = pic);
			pane_animator_map[pic->name] = pic;
		}
		// TODO: these "bnd1" sections seem to tell the wii the viewport for different screen sizes
		else if (header.magic == "pan1"/* || header.magic == "bnd1"*/)
		{
			pane_stack.top()->push_back(last_pane = new Pane(file));
			pane_animator_map[last_pane->name] = last_pane;
		}
		else if (header.magic == "pas1")
		{
			if (last_pane)
				pane_stack.push(&last_pane->panes);
		}
		else if (header.magic == "pae1")
		{
			if (pane_stack.size() > 1)
				pane_stack.pop();
		}
		else if (header.magic == "grp1")
		{
			char read_name[0x11] = {};
			file.read(read_name, 0x10);

			Group& group_ref = (*group_stack.top())[read_name];

			u16 sub_count;
			file >> BE >> sub_count;
			file.seekg(2, std::ios::cur);

			while (sub_count--)
			{
				char read_name[0x11];
				file.read(read_name, 0x10);
				group_ref.panes.push_back(read_name);
			}

			last_group = &group_ref;
		}
		else if (header.magic == "grs1")
		{
			if (last_group)
				group_stack.push(&last_group->groups);
		}
		else if (header.magic == "gre1")
		{
			if (group_stack.size() > 1)
				group_stack.pop();
		}
		else
		{
			std::cout << "UNKNOWN SECTION: ";
			std::cout.write((char*)header.magic.data, 4) << '\n';
			//std::cin.get();
		}
	}
}

Layout::~Layout()
{
	ForEach(panes, [](Pane* pane)
	{
		delete pane;
	});

	ForEach(materials, [](Material* material)
	{
		delete material;
	});

	ForEach(textures, [](Texture* texture)
	{
		delete texture;
	});
}

void Layout::Render() const
{
	glLoadIdentity();

	glOrtho(-width, 0, -height, 0, -1000.f, 1000.f);

	if (centered)
		glTranslatef(-width / 2, -height / 2, 0.f);

	// usually there is only one root pane, probably always
	ForEach(panes, [&](Pane* pane)
	{
		pane->Render(0xff);	// fully opaque
	});
}

void Layout::SetFrame(FrameNumber frame_number)
{
	frame_current = frame_number;

	ForEach(panes, [&](Pane* pane)
	{
		pane->SetFrame(frame_number);
	});

	ForEach(materials, [&](Material* material)
	{
		material->SetFrame(frame_number);
	});
}

void Layout::AdvanceFrame()
{
	++frame_current;

	// should i just use == ?
	if (frame_current >= frame_loop_end)
		frame_current = frame_loop_start;

	SetFrame(frame_current);
}

void Layout::SetLanguage(const std::string& language)
{
	// TODO: i'd like an empty language to unhide everything

	// hide panes of non-matching languages
	ForEach(groups["RootGroup"].groups, [&language, this](const std::pair<const std::string&, const Group&> group)
	{
		// some hax, there are some odd "Rso0" "Rso1" groups that shouldn't be hidden
		// only the 3 character language groups should be
		if (group.first != language && group.first.length() == 3)
		{
			ForEach(group.second.panes, [&](const std::string& pane)
			{
				Pane* const found = FindPane(pane);
				if (found)
					found->SetHide(true);
			});
		}
	});

	// unhide panes of matching language, some banners list language specific panes in multiple language groups
	ForEach(groups["RootGroup"].groups[language].panes, [&](const std::string& pane)
	{
		Pane* const found = FindPane(pane);
		if (found)
			found->SetHide(false);
	});
}

Pane* Layout::FindPane(const std::string& find_name)
{
	Pane* found = nullptr;

	ForEach(panes, [&](Pane* pane)
	{
		if (!found)
			found = pane->FindPane(find_name);	// TODO: oh noes, can't break out of this lambda loop
	});

	return found;
}

Material* Layout::FindMaterial(const std::string& find_name)
{
	Material* found = nullptr;

	ForEach(materials, [&](Material* material)
	{
		if (!found && find_name == material->name)
			found = material;	// TODO: oh noes, can't break out of this lambda loop
	});

	return found;
}

}
