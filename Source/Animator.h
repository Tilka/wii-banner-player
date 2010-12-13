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

#ifndef _ANIMATOR_H_
#define _ANIMATOR_H_

#include <list>
#include <map>

#include "Types.h"

typedef float FrameNumber;

enum KEYFRAME_TYPE
{
	RLPA,
	RLTS,
	RLVI,
	RLVC,
	RLMC,
	RLTP,
	RLIM
};

struct KeyFrameType
{
	KeyFrameType() {}

	KeyFrameType(u8 _type, u8 _index)
		: type(_type)
		, index(_index)
	{}

	bool operator<(const KeyFrameType& rhs) const
	{
		return memcmp(this, &rhs, sizeof(*this)) < 0;
	}

	u8 type, index;
};

struct KeyFrame
{
	KeyFrame() {}

	KeyFrame(float _value, float _blend)
		: value(_value)
		, blend(_blend)
	{}

	float value, blend;
};

class Animator
{
public:
	virtual void SetFrame(FrameNumber frame);

	Animator& operator+=(const Animator& rhs);

//private:
	virtual void ProcessRLPA(u8 index, float value) {}	// Pane Animation
	virtual void ProcessRLTS(u8 index, float value) {}	// Texture Scale/Rotate/Translate
	virtual void ProcessRLVI(bool value) {}	// Visibility
	virtual void ProcessRLVC() {}	// Vertex Color
	virtual void ProcessRLMC() {}	// Material Color
	virtual void ProcessRLTP() {}	// Texture Pallete
	virtual void ProcessRLIM() {}	// 

	std::map<KeyFrameType, std::map<FrameNumber, KeyFrame> > key_frames;

	u8 is_material;

	std::string name;
};

#endif
