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

// multiply 2 colors
// assumes u8s, takes any type to avoid multiple conversions
template <typename C1, typename C2>
inline u8 MultiplyColors(C1 c1, C2 c2)
{
	return (u16)c1 * c2 / 0xff;
}

namespace WiiBanner
{

class Pane : public Animator
{
public:
	typedef Animator Base;

	Pane(std::istream& file);
	virtual ~Pane();

	void Render(u8 render_alpha) const;
	void SetFrame(FrameNumber frame);

	std::vector<Pane*> panes;	// TODO: rename children or something?

	float width, height;

	bool hide;	// used by the groups
	u8 origin;

private:
	virtual void Draw(u8 render_alpha) const {};

	u8 visible;
	u8 alpha;

	struct
	{
		float x, y, z;
	} translate, rotate;

	struct
	{
		float x, y;
	} scale;

protected:
	void ProcessHermiteKey(const KeyType& type, float value);
	void ProcessStepKey(const KeyType& type, StepKeyHandler::KeyData data);
};

}

#endif
