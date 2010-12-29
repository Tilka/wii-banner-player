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
#include "Sound.h"

#include <gl/glew.h>

namespace WiiBanner
{

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

// load keyframes from a brlan file
// func is a callback func which is passed all the read Animators
template <typename F>
FrameNumber LoadAnimators(std::istream& file, F func)
{
	const std::streamoff file_start = file.tellg();

	u16 frame_count; // number of frames

	// read header
	FourCC magic; // "RLAN"
	u16 endian; // always 0xFEFF
	u16 version; // always 0x0008

	file >> magic >> BE >> endian >> version;

	if (magic != "RLAN"
		|| endian != 0xFEFF
		|| version != 0x008
		)
		return 0;	// bad header


	u32 file_size; // size of entire file including header
	u16 offset; // offset to the first section, from start of file
	u16 section_count;

	file >> BE >> file_size >> offset >> section_count;

	// only a single pa*1 section is currently supported
	if (section_count > 1)
		section_count = 1;

	// seek to header of first section
	file.seekg(file_start + offset, std::ios::beg);

	// read each section
	SectionHeader header(file);
	while (section_count--)
	{
		file += header;
		file >> header;

		if (header.magic == "pai1")
		{
			u8 flags;
			u8 padding;
			u16 timg_count; // ?
			u16 animator_count;
			u32 entry_offset;

			file >> BE >> frame_count >> flags
				>> padding >> timg_count >> animator_count;

			// extra padding if bit 25 is set, idk why
			// TODO: never true
			if (flags & (1 << 25))
			{
				//file.seekg(4);

				float pad;
				file >> BE >> pad;
				std::cout << "pad: " << pad << '\n';
			}

			file >> BE >> entry_offset;
			file.seekg(header.start + entry_offset);

			// read each animator
			ReadOffsetList(file, animator_count, header.start, [&]()
			{
				const std::streamoff origin = file.tellg();

				Animator animator;

				{
				char read_name[21] = {};	// this name must be defined in the brlyt file
				file.read(read_name, 20);
				animator.name = read_name;
				}

				u8 tag_count;
				u8 is_material;
				u16 offset;

				file >> BE >> tag_count >> is_material >> offset;

				ReadOffsetList(file, tag_count, origin, [&]()
				{
					const std::streamoff origin = file.tellg();

					FourCC magic;
					u8 entry_count;

					file >> magic >> entry_count;
					file.seekg(3, std::ios::cur);	// some padding

					KeyFrameTag tag = (KeyFrameTag)-1;
					if (magic == "RLPA")
						tag = RLPA;
					else if (magic == "RLTS")
						tag = RLTS;
					else if (magic == "RLVI")
						tag = RLVI;
					else if (magic == "RLVC")
						tag = RLVC;
					else if (magic == "RLMC")
						tag = RLMC;
					else if (magic == "RLTP")
						tag = RLTP;
					else if (magic == "RLIM")
						tag = RLIM;
					else
					{
						std::cout << "unknown frame tag: ";
						std::cout.write((char*)magic.data, 4) << '\n';
						//std::cin.get();
					}

					ReadOffsetList(file, entry_count, origin, [&]
					{
						u8 index;
						u8 target;
						u8 data_type;
						u8 pad;
						u16 frame_count;
						u16 pad1;
						u32 offset;

						file >> BE >> index >> target >> data_type >> pad
							>> frame_count >> pad1 >> offset;

						const KeyType frame_type(tag, index, target);

						switch (data_type)
						{
							// step key frame
						case 0x01:
							animator.step_keys[frame_type].Load(file, frame_count);
							break;

							// hermite key frame
						case 0x02:
							animator.hermite_keys[frame_type].Load(file, frame_count);
							break;

						default:
							std::cout << "UNKNOWN FRAME DATA TYPE!!\n";
							break;
						}
					});
				});

				// pass the read animator to the callback function
				func(animator, is_material);
			});
		}
	}

	return frame_count;
}

Banner::Banner(const std::string& filename)
	: frame_current(0)
	, frame_loop_start(0)
	, frame_loop_end(0)
	, width(1), height(1)	// 1 is stupid, but no /0 errors this way :p
	, centered(0)
{
	std::map<std::string, Pane*> pane_animator_map;
	std::map<std::string, Material*> mate_animator_map;

	Pane* last_pane = NULL;
	std::stack<std::vector<Pane*>*> pane_stack;
	pane_stack.push(&panes);

	// temporary
	struct Group
	{
		std::list<std::string> panes;
		std::map<std::string, Group> groups;
	};

	std::map<std::string, Group> groups;

	Group* last_group = NULL;
	std::stack<std::map<std::string, Group>*> group_stack;
	group_stack.push(&groups);

	// bunch of crap to parse/decompress archives multiple times

	std::ifstream opening_arc_file(filename, std::ios::binary | std::ios::in);
	opening_arc_file.seekg(0x600);	// skip the header
	DiscIO::CARCFile opening_arc(opening_arc_file);

	// This is probably the wrong way to do it anyways. Should just
	// read the filesystem and parse what's available. Also, things aren't
	// necessarily in LZ77
	const auto banner_offset = opening_arc.GetFileOffset("meta/banner.bin");
	std::cout << "banner.bin offset is: " << banner_offset << '\n';
	const auto icon_offset = opening_arc.GetFileOffset("meta/icon.bin");
	std::cout << "icon.bin offset is: " << icon_offset << '\n';
	const auto sound_offset = opening_arc.GetFileOffset("meta/sound.bin");
	std::cout << "sound.bin offset is: " << sound_offset << '\n';

	// seek past IMD5 header
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

				//std::cout << '\t' << textures.size() << ' ' << fname << '\n';

				file.clear();
				file.seekg(texture_offset, std::ios::beg);
				textures.push_back(new Texture(file));

			}, 4);

			std::cout << "loaded " << textures.size() << " textures\n";
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

			std::cout << "loaded " << materials.size() << " materials\n";
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

	// load animation files containing key frames
	auto const add_animators = [&,this](Animator& an, u8 is_material)
	{
		Animator* const anim = is_material ?
			static_cast<Animator*>(mate_animator_map[an.name]) :
			static_cast<Animator*>(pane_animator_map[an.name]);

		if (anim)
			anim->CopyFrames(an, frame_loop_start);
	};


	auto const brlan_start_offset = banner_arc.GetFileOffset("arc/anim/Banner_Start.brlan");
	if (brlan_start_offset)
	{
		// banner uses 2 brlan files, a starting one and a looping one
		auto const brlan_loop_offset = banner_arc.GetFileOffset("arc/anim/Banner_Loop.brlan");	

		file.seekg(brlan_start_offset, std::ios::beg);
		frame_loop_start = LoadAnimators(file, add_animators);

		file.seekg(brlan_loop_offset, std::ios::beg);
		frame_loop_end = frame_loop_start + LoadAnimators(file, add_animators);
	}
	else
	{
		// banner uses a single repeating brlan
		auto const brlan_lone_offset = banner_arc.GetFileOffset("arc/anim/Banner.brlan");	

		file.seekg(brlan_lone_offset, std::ios::beg);
		frame_loop_end = LoadAnimators(file, add_animators);
	}

	// update everything for frame 0
	SetFrame(frame_current);

	// print the group layout
	//ForEach(groups, [&](Group& group)
	//{
	//	group.Print(0);
	//});

	// TEMPORARY fixed language
	static const char active_language[] = "ENG";

	// hide panes depending on language
	ForEach(groups["RootGroup"].groups, [&](const std::pair<const std::string&, const Group&> group)
	{
		if (group.first != active_language)
		{
			ForEach(group.second.panes, [&](const std::string& pane)
			{
				auto const pane_it = pane_animator_map.find(pane);

				if (pane_animator_map.end() != pane_it)
					pane_it->second->hide = true;
			});
		}

		// need to unhide the correct language, some banners list language specific panes in multiple language groups
		ForEach(groups["RootGroup"].groups[active_language].panes, [&](const std::string& pane)
		{
			auto const pane_it = pane_animator_map.find(pane);

			if (pane_animator_map.end() != pane_it)
				pane_it->second->hide = false;
		});
	});

	//// for testing
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
	//	std::cout << "rotate: z:\t" << pane->rotate.z << '\n';

	//	std::cout << '\n';
	//}

	// AUDIO!!
	// seek past IMD5 header
	opening_arc_file.seekg(0x600 + sound_offset + 32, std::ios::beg);
	sound.Open(opening_arc_file);
}

void Banner::SetFrame(FrameNumber frame_number)
{
	ForEach(panes, [&](Pane* pane)
	{
		pane->SetFrame(frame_number);
	});

	ForEach(materials, [&](Material* material)
	{
		material->SetFrame(frame_number);
	});
}

void Banner::Render()
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

void Banner::AdvanceFrame()
{
	++frame_current;

	// should i just use == ?
	if (frame_current >= frame_loop_end)
		frame_current = frame_loop_start;

	SetFrame(frame_current);
}

}
