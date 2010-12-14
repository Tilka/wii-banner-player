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

#include <map>

#include "Types.h"

typedef float FrameNumber;

enum FRAME_TAG
{
	RLPA,
	RLTS,
	RLVI,
	RLVC,
	RLMC,
	RLTP,
	RLIM
};

struct FrameType
{
	FrameType(FRAME_TAG _tag, u8 _type, u8 _index)
		: tag(_tag)
		, type(_type)
		, index(_index)
	{}

	bool operator<(const FrameType& rhs) const
	{
		return memcmp(this, &rhs, sizeof(*this)) < 0;
	}

	const FRAME_TAG tag;
	const u8 type, index;
};

class Animator;

class StaticFrameHandler
{
public:
	void Load(std::istream& file, u16 count);

	// TODO: std::pair is silly, make a struct
	typedef std::pair<u8, u8> Frame;

	Frame GetFrame(FrameNumber frame_number) const;
	void CopyFrames(const StaticFrameHandler& kf, FrameNumber frame_offset);

private:
	void Process(FrameNumber frame, Animator& animator) const;

	std::map<FrameNumber, Frame> frames;
};

class KeyFrameHandler
{
public:
	void Load(std::istream& file, u16 count);

	typedef float Frame;

	Frame GetFrame(FrameNumber frame_number) const;
	void CopyFrames(const KeyFrameHandler& kf, FrameNumber frame_offset);

private:
	void Process(FrameNumber frame, Animator& animator) const;

	// TODO: handle the "blend" float
	std::map<FrameNumber, Frame> frames;
};

class Animator
{
public:
	virtual void SetFrame(FrameNumber frame);

	void CopyFrames(Animator& rhs, FrameNumber frame_offset);

//private:
	virtual void ProcessRLPA(u8 index, float value) {}	// Pane Animation
	virtual void ProcessRLTS(u8 index, float value) {}	// Texture Scale/Rotate/Translate
	virtual void ProcessRLVI(u8 value) {}	// Visibility
	virtual void ProcessRLVC(u8 index, u8 value) {}	// Vertex Color
	virtual void ProcessRLMC(u8 index, u8 value) {}	// Material Color
	virtual void ProcessRLTP() {}	// Texture Pallete
	virtual void ProcessRLIM() {}	// 

	std::map<FrameType, StaticFrameHandler> static_frames;
	std::map<FrameType, KeyFrameHandler> key_frames;

	std::string name;
};

#endif
