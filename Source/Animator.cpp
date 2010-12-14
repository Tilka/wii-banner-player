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

#include "Animator.h"

float BlendValue(FrameNumber frame, const std::map<FrameNumber, KeyFrame>& key_frames)
{
	// find the current keyframe, or the one after it
	auto next = key_frames.lower_bound(frame);
	
	// current frame is higher than any keyframe, use the last keyframe
	if (key_frames.end() == next)
		--next;

	auto prev = next;

	// if this is not the current frame and not the first keyframe, use the previous one
	if (frame < prev->first && key_frames.begin() != prev)
		--prev;

	// TODO: use "blend", dunno how it works

	// clamp
	frame = std::max(prev->first, std::min(next->first, frame));

	if (next->first == prev->first)
	{
		return prev->second.value;
	}
	else
	{
		const float intrpl = (frame - prev->first) / (next->first - prev->first);
		return prev->second.value * (1.f - intrpl) + next->second.value * intrpl;
	}
}

void Animator::SetFrame(FrameNumber frame)
{
	ForEach(key_frames, [&, this](const std::pair<KeyFrameType, std::map<FrameNumber, KeyFrame> >& kf)
	{
		const float value = BlendValue(frame, kf.second);

		switch (kf.first.type)
		{
		case RLPA:
			ProcessRLPA(kf.first.index, value);
			break;

		case RLTS:
			ProcessRLTS(kf.first.index, value);
			break;

		case RLVI:
			ProcessRLVI(value > 0.5f);
			break;

		case RLVC:
			ProcessRLVC(kf.first.index, value);
			break;

		case RLMC:
			ProcessRLMC();
			break;

		case RLTP:
			ProcessRLTP();
			break;

		case RLIM:
			ProcessRLIM();
			break;
		}
	});
}

Animator& Animator::operator+=(const Animator& rhs)
{
	ForEach(rhs.key_frames, [this](const std::pair<KeyFrameType, std::map<FrameNumber, KeyFrame> >& kf)
	{
		key_frames[kf.first].insert(kf.second.begin(), kf.second.end());
	});

	return *this;
}
