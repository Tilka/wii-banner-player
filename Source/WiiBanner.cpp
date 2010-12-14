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

#include "WiiBanner.h"

#include "FileHandlerARC.h"

#include "LZ77.h"

struct SectionHeader
{
	SectionHeader(std::istream& file)
		: start(file.tellg())
		, size(0)
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
inline std::istream& operator+=(std::istream& lhs, SectionHeader& rhs)
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

// load keyframes from a brlan file
// func is a callback func which is passed all the read Animators
// TODO: remove the frame_offset, pretty sure it's not needed
template <typename F>
FrameNumber LoadAnimators(std::istream& file, F func, FrameNumber frame_offset = 0)
{
	const std::streamoff file_start = file.tellg();

	u16 frame_count; // number of frames

	// read header
	FourCC magic; // "RLAN" in ASCII.
	u16 endian; // Always 0xFEFF. Tells endian.
	u16 version; // Always 0x0008. Version of brlan format
	u32 file_size; // Size of whole file, including the header.
	u16 offset; // The offset to the pa*1 header from the start of file.
	u16 section_count; // How many pa*1 sections there are

	file >> magic >> BE >> endian >> version
		>> file_size >> offset >> section_count;

	if (magic != "RLAN"
		|| endian != 0xFEFF
		|| version != 0x008
		|| section_count != 1 // only a single pa*1 section is currently supported
		)
		return 0;	// bad header

	// seek to pai1_header
	file.seekg(file_start + offset, std::ios::beg);

	SectionHeader header(file);
	while (section_count--)
	{
		file += header;
		file >> header;

		if (header.magic == "pai1")
		{
			u8 flags; // Flags
			u8 padding; // Padding
			u16 num_timgs; // Number of timgs?
			u16 num_entries; // Number of tags in the brlan.
			u32 entry_offset; // Offset to entries. (Relative to start of pai1 header.)

			file >> BE >> frame_count >> flags
				>> padding >> num_timgs >> num_entries;

			// extra padding if bit 25 is set, idk why
			if (flags & (1 << 25))
			{
				//file.seekg(4);

				float pad;
				file >> BE >> pad;
				std::cout << "pad: " << pad << '\n';
			}

			file >> BE >> entry_offset;
			file.seekg(header.start + entry_offset);

			// read entries
			ReadOffsetList(file, num_entries, header.start, [&]()
			{
				const std::streamoff origin = file.tellg();

				Animator animator;

				char name[21] = {}; // Name of the BRLAN entry. (Must be defined in the BRLYT)
				u8 num_tags;
				u16 offset;	// TODO: not sure if offset

				// read the entry
				file.read(name, 20) >> BE >> num_tags >> animator.is_material >> offset;

				animator.name = name;
			
				std::cout << "entry: " << name << '\n';

				ReadOffsetList(file, num_tags, origin, [&]()
				{
					const std::streamoff origin = file.tellg();

					FourCC magic;
					u8 entry_count; // How many entries in this chunk.

					file >> magic >> entry_count;
					file.seekg(3, std::ios::cur);	// some padding

					u8 type = -1;
					if (magic == "RLPA")
						type = RLPA;
					else if (magic == "RLTS")
						type = RLTS;
					else if (magic == "RLVI")
						type = RLVI;
					else if (magic == "RLVC")
						type = RLVC;
					else if (magic == "RLMC")
						type = RLMC;
					else if (magic == "RLTP")
						type = RLTP;

					std::cout << "\ttag: ";
					std::cout.write((char*)magic.data, 4) << '\n';

					ReadOffsetList(file, entry_count, origin, [&]
					{
						u8 unk;
						u8 index;
						u16 data_type; // Every case has been 0x0200 // 0x0100 for pairs
						u16 coord_count; // How many coordinates.
						u16 pad1; // All cases I've seen is zero.
						u32 offset; // Offset to tag data

						file >> BE >> unk >> index >> data_type
							>> coord_count >> pad1 >> offset;

						std::cout << "\t\ttagentry: index: " << (int)index << " coord_count: " << coord_count << '\n';

						float frame;

						// read coords
						if (0x0200 == data_type)
						{
							while (coord_count--)
							{
								float value, blend;

								file >> BE >> frame >> value >> blend;

								if (animator.name == "PicLogoShine")
								std::cout << "\t\t\tcoord: frame: " << frame << " value: " << value << " blend: " << blend << '\n';

								// add a new keyframe to the latest animator
								animator.key_frames[KeyFrameType(type, index)][frame + frame_offset] = KeyFrame(value, blend);
							}
						}
						else
						{
							while (coord_count--)
							{
								u16 value;
				
								file >> BE >> frame >> value;
								file.seekg(2, std::ios::cur);

								if (animator.name == "PicLogoShine")
								std::cout << "\t\t\tcoord: frame: " << frame << " value: " << (int)value << '\n';

								// add a new keyframe to the latest animator
								animator.key_frames[KeyFrameType(type, index)][frame + frame_offset] = KeyFrame(value, 0);
							}
						}
					});
				});

				// pass the read animator to the callback function
				func(animator);
			});
		}
	}

	return frame_count;
}

WiiBanner::WiiBanner(const std::string& path)
	: frame_current(0)
	, frame_loop_start(0)
	, frame_loop_end(300)	// hax
{
	std::map<std::string, Animator*> pane_animator_map;
	std::map<std::string, Material*> mate_animator_map;

	PaneHolder* last_pane;
	std::stack<std::vector<Pane*>*> pane_stack;
	pane_stack.push(&panes);

	// bunch of crap to parse/decompress archives multiple times

	std::ifstream opening_arc_file(path, std::ios::binary | std::ios::in);
	opening_arc_file.seekg(0x600);	// skip the header
	DiscIO::CARCFile opening_arc(opening_arc_file);

	const auto banner_offset = opening_arc.GetFileOffset("meta/banner.bin");
	std::cout << "banner.bin offset is: " << banner_offset << '\n';

	// there is some 32 byte header
	opening_arc_file.seekg(0x600 + banner_offset + 32, std::ios::beg);

	// LZ77 decompress "banner.bin"
	LZ77Decompressor decomp(opening_arc_file);
	DiscIO::CARCFile banner_arc(decomp.GetStream());

	const auto brlyt_offset = banner_arc.GetFileOffset("arc/blyt/Banner.brlyt");
	std::cout << "Banner.brlyt offset is: " << brlyt_offset << '\n';

	std::istream& file = decomp.GetStream();
	file.seekg(brlyt_offset, std::ios::beg);

	// read header
	FourCC magic; // RLYT
	u16 endian; // 0xFEFF
	u16 version; // 0x0008
	u32 filesize; // The filesize of the brlyt.
	u16 offset; // Offset to the lyt1 section.
	u16 section_count; // Number of sections.

	file >> magic >> BE >> endian >> version
		>> filesize >> offset >> section_count;

	if (magic != "RLYT"
		|| endian != 0xFEFF
		|| version != 0x008
		)
		return;	// bad header

	file.seekg(brlyt_offset + offset, std::ios::beg);

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
			u16 count;
			u16 offset;

			file >> BE >> count >> offset;

			ReadOffsetList(file, count, file.tellg(), [&]
			{
				std::string fname;
				std::getline(file, fname, '\0');

				const auto texture_offset = banner_arc.GetFileOffset("arc/timg/" + fname);

				std::cout << '\t' << textures.size() << ' ' << fname << '\n';

				file.clear();
				file.seekg(texture_offset, std::ios::beg);
				textures.push_back(new Texture(file));

			}, 4);

			std::cout << "\tloaded " << textures.size() << " textures\n";
			//std::cin.get();
		}
		else if (header.magic == "mat1")
		{
			// load materials
			u16 count; // num materials
			u16 offset; // Offset to list start. Always zero

			file >> BE >> count >> offset;

			ReadOffsetList(file, count, header.start, [&]
			{
				Material* const mate = new Material(file, textures);
				materials.push_back(mate);
				mate_animator_map[materials.back()->name] = mate;
			});

			std::cout << "\tloaded " << materials.size() << " materials\n";
		}
		else if (header.magic == "pan1")
		{
			pane_stack.top()->push_back(last_pane = new PaneHolder(file));
			pane_animator_map[last_pane->name] = last_pane;
		}
		else if (header.magic == "pic1")
		{
			Pane* const pane = new Picture(file, materials);
			pane_stack.top()->push_back(pane);
			pane_animator_map[pane->name] = pane;
		}
		else if (header.magic == "pas1")
		{
			pane_stack.push(&last_pane->panes);
		}
		else if (header.magic == "pae1")
		{
			pane_stack.pop();
		}
	}

	// load animation files containing key frames
	auto const add_animators = [&,this](const Animator& an)
	{
		// TODO: some safety checks

		Animator* const anim = an.is_material ?
			mate_animator_map[an.name] :
			pane_animator_map[an.name];

		if (anim)
			*anim += an;
	};

	auto const brlan_start_offset = banner_arc.GetFileOffset("arc/anim/Banner_Start.brlan");
	auto const brlan_loop_offset = banner_arc.GetFileOffset("arc/anim/Banner_Loop.brlan");

	file.seekg(brlan_start_offset, std::ios::beg);
	frame_loop_start = LoadAnimators(file, add_animators);
	file.seekg(brlan_loop_offset, std::ios::beg);
	frame_loop_end = frame_loop_start + LoadAnimators(file, add_animators, frame_loop_start);

	SetFrame(frame_current);

	// print the pane layout
	ForEach(panes, [&](const Pane* pane)
	{
		pane->Print(0);
	});

	//std::string pname;
	//while (std::getline(std::cin, pname) && pname.size())
	//{
	//	Pane* pane = (Pane*)pane_animator_map[pname];
	//	if (!pane)
	//		continue;

	//	std::cout << "x,y:\t\t" << pane->translate.x << ", " << pane->translate.y << '\n';
	//	std::cout << "w,h:\t\t" << pane->width << ", " << pane->height << '\n';
	//	std::cout << "scale x,y:\t" << pane->scale.x << ", " << pane->scale.y << '\n';
	//	std::cout << "origin:\t\t" << (pane->origin % 3) << ", " << (pane->origin / 3) << '\n';

	//	std::cout << '\n';
	//}
}

void WiiBanner::SetFrame(FrameNumber frame)
{
	ForEach(panes, [&](Pane* pane)
	{
		pane->SetFrame(frame);
	});

	ForEach(materials, [&](Material* material)
	{
		material->SetFrame(frame);
	});
}

void WiiBanner::Render()
{
	glLoadIdentity();
	gluOrtho2D(-width, 0, -height, 0);

	if (centered)
		glTranslatef(-width / 2, -height / 2, 0);

	//std::cout << (int)centered << '\n';

	// usually there is only one root pane, probably always
	ForEach(panes, [&](Pane* pane)
	{
		pane->Render();
	});
}

void WiiBanner::AdvanceFrame()
{
	++frame_current;

	// > or >= ?
	if (frame_current > frame_loop_end)
		frame_current = frame_loop_start;

	SetFrame(frame_current);
}
