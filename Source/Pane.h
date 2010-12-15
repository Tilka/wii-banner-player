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

#ifndef _PANE_H_
#define _PANE_H_

#include "Types.h"

#include <vector>

#include "Animator.h"

// TODO: put opengl code elsewhere
#include <gl/glew.h>

class Pane : public Animator
{
public:
	virtual ~Pane() {}

	Pane(std::istream& file);

public:
	u8 visible;
	u8 origin;
	u8 alpha;		// used?

	bool hide;	// used by the groups

	struct XYZ
	{
		float x, y, z;
	} translate, rotate;

	struct
	{
		float x, y;
	} scale;

	float width, height;

	void Render() const;

	virtual void Print(unsigned int level) const = 0;

private:
	virtual void Draw() const = 0;

	void ProcessRLPA(u8 index, float value);
	void ProcessRLVI(u8 value);
};

class PaneHolder : public Pane
{
public:
	PaneHolder::PaneHolder(std::istream& file) : Pane(file) {}

	void SetFrame(FrameNumber frame);

//protected:
	void Draw() const;

	void Print(unsigned int level) const
	{
		std::cout << std::string(level, '\t') << "pane: " << name << '\n';

		ForEach(panes, [&](Pane* pane)
		{
			pane->Print(level + 1);
		});
	}

	std::vector<Pane*> panes;

	GLuint framebuffer;
};

#endif
