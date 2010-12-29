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

namespace WiiBanner
{

typedef float FrameNumber;

enum KeyFrameTag
{
	RLPA,
	RLTS,
	RLVI,
	RLVC,
	RLMC,
	RLTP,
	RLIM
};

struct KeyType
{
	KeyType(KeyFrameTag _tag, u8 _index, u8 _target)
		: tag(_tag)
		, index(_index)
		, target(_target)
	{}

	bool operator<(const KeyType& rhs) const
	{
		return memcmp(this, &rhs, sizeof(*this)) < 0;
	}

	const KeyFrameTag tag;
	const u8 index, target;
};

class Animator;

class StepKeyHandler
{
public:
	void Load(std::istream& file, u16 count);

	union KeyData
	{
		struct
		{
			u8 data1, data2;
		};

		u16 value;
	};

	KeyData GetFrame(FrameNumber frame_number) const;
	void CopyFrames(const StepKeyHandler& kf, FrameNumber frame_offset);

private:
	void Process(FrameNumber frame, Animator& animator) const;

	std::map<FrameNumber, KeyData> keys;
};

class HermiteKeyHandler
{
public:
	void Load(std::istream& file, u16 count);

	struct KeyData
	{
		float value, slope;
	};

	float GetFrame(FrameNumber frame_number) const;
	void CopyFrames(const HermiteKeyHandler& kf, FrameNumber frame_offset);

private:
	void Process(FrameNumber frame, Animator& animator) const;

	std::multimap<FrameNumber, KeyData> keys;
};

class Animator
{
public:
	virtual void SetFrame(FrameNumber frame);

	void CopyFrames(Animator& rhs, FrameNumber frame_offset);

//protected:
	std::string name;

	std::map<KeyType, StepKeyHandler> step_keys;
	std::map<KeyType, HermiteKeyHandler> hermite_keys;

//private:
	virtual void ProcessHermiteKey(const KeyType& type, float value);
	virtual void ProcessStepKey(const KeyType& type, StepKeyHandler::KeyData data);
};

}

#endif
