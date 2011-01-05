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

#ifndef _LAYOUT_H_
#define _LAYOUT_H_

#include "Pane.h"
#include "Material.h"
#include "Texture.h"
#include "Font.h"

#include "Types.h"

// TODO: i would like to remove this dependency in this file
#include "FileHandlerARC.h"

#include <map>
#include <list>
#include <vector>

namespace WiiBanner
{

class Layout
{
public:
	Layout(std::istream& file);
	~Layout();

	void Render() const;

	FrameNumber GetFrame() const { return frame_current; }
	void SetFrame(FrameNumber frame_number);
	void AdvanceFrame();

	void SetLoopStart(FrameNumber loop_start) { frame_loop_start = loop_start; }
	void SetLoopEnd(FrameNumber loop_end) { frame_loop_end = loop_end; }

	float GetWidth() const { return width; }
	float GetHeight() const { return height; }

	void SetLanguage(const std::string& language);

	Pane* FindPane(const std::string& name);
	Material* FindMaterial(const std::string& name);

//private:
	struct Group
	{
		std::map<std::string, Group> groups;
		std::list<std::string> panes;
	};

	std::vector<Pane*> panes;
	std::vector<Material*> materials;
	std::vector<Texture*> textures;
	std::vector<Font*> fonts;

	std::map<std::string, Group> groups;

private:

	FrameNumber frame_current, frame_loop_start, frame_loop_end;

	float width, height;
	u8 centered;
};

}

#endif
