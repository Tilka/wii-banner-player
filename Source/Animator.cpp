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

//template <typename T>
//inline T BlendValues(T p1, T p2, float intrpl)
//{
//	// will also work with u8 types this way
//	if (p2 > p1)
//	{
//		const T diff = p2 - p1;
//		return p1 + T(diff * intrpl);
//	}
//	else
//	{
//		const T diff = p1 - p2;
//		return p1 - T(diff * intrpl);
//	}
//}

void Animator::SetFrame(FrameNumber frame_number)
{
	ForEach(key_frames, [=](const std::pair<const FrameType&, const KeyFrameHandler&> frame_handler)
	{
		const auto frame_value = frame_handler.second.GetFrame(frame_number);

		const auto& frame_type = frame_handler.first;

		switch (frame_handler.first.tag)
		{
		case RLPA:
			ProcessRLPA(frame_type.index, frame_value);
			break;

		case RLTS:
			ProcessRLTS(frame_type.type, frame_type.index, frame_value);
			break;

		case RLVC:
			ProcessRLVC(frame_type.index, (u8)frame_value);
			break;

		case RLMC:
			ProcessRLMC(frame_type.index, (u8)frame_value);
			break;
		}
	});

	ForEach(static_frames, [=](const std::pair<const FrameType&, const StaticFrameHandler&> frame_handler)
	{
		auto const frame_data = frame_handler.second.GetFrame(frame_number);

		switch (frame_handler.first.tag)
		{
		case RLVI:
			ProcessRLVI(frame_data.second);
			break;

		case RLTP:
			//ProcessRLTP(frame_data.second);
			break;

		case RLIM:
			//ProcessRLIM(frame_data.second);
			break;
		}
	});
}

void StaticFrameHandler::Load(std::istream& file, u16 count)
{
	while (count--)
	{
		FrameNumber frame;
		file >> BE >> frame;

		auto& pair = frames[frame];
		file >> BE >> pair.first >> pair.second;
		file.seekg(2, std::ios::cur);	// these bytes important? :p

		//std::cout << "\t\t\t" "frame: " << frame << ' ' << pair.first << " " << pair.second << '\n';
	}
}

void KeyFrameHandler::Load(std::istream& file, u16 count)
{
	while (count--)
	{
		FrameNumber frame;
		file >> BE >> frame;

		file >> BE >> frames[frame];
		file.seekg(4, std::ios::cur);	// skipping the "blend" value, dunno how to use it :/

		//std::cout << "\t\t\t" "frame: " << frame << ' ' << frames[frame] << '\n';
	}
}

StaticFrameHandler::Frame StaticFrameHandler::GetFrame(FrameNumber frame_number) const
{
	// assuming not empty, a safe assumption currently

	auto frame_it = frames.lower_bound(frame_number);

	// current frame is higher than any keyframe, use the last keyframe
	if (frames.end() == frame_it)
		--frame_it;

	// if this is after the current frame and not the first keyframe, use the previous one
	if (frame_number < frame_it->first && frames.begin() != frame_it)
		--frame_it;

	return frame_it->second;
}

KeyFrameHandler::Frame KeyFrameHandler::GetFrame(FrameNumber frame_number) const
{
	// assuming not empty, a safe assumption currently

	// find the current keyframe, or the one after it
	auto next = frames.lower_bound(frame_number);
	
	// current frame is higher than any keyframe, use the last keyframe
	if (frames.end() == next)
		--next;

	auto prev = next;

	// if this is after the current frame and not the first keyframe, use the previous one
	if (frame_number < prev->first && frames.begin() != prev)
		--prev;

	if (next->first == prev->first)
	{
		// same 2 frames, return the first one
		return prev->second;
	}
	else
	{
		// different frames, blend them together

		// clamp
		frame_number = std::max(prev->first, std::min(next->first, frame_number));
		const float intrpl = (frame_number - prev->first) / (next->first - prev->first);

		return prev->second + (next->second - prev->second) * intrpl;
	}
}

void StaticFrameHandler::CopyFrames(const StaticFrameHandler& fh, FrameNumber frame_offset)
{
	ForEach(fh.frames, [=](const std::pair<const FrameNumber, const Frame&> frame)
	{
		frames[frame.first + frame_offset] = frame.second;
	});
}

void KeyFrameHandler::CopyFrames(const KeyFrameHandler& fh, FrameNumber frame_offset)
{
	ForEach(fh.frames, [=](const std::pair<const FrameNumber, const Frame&> frame)
	{
		frames[frame.first + frame_offset] = frame.second;
	});
}

void Animator::CopyFrames(Animator& rhs, FrameNumber frame_offset)
{
	ForEach(rhs.key_frames, [=](const std::pair<const FrameType&, const KeyFrameHandler&> kf)
	{
		key_frames[kf.first].CopyFrames(kf.second, frame_offset);
	});

	ForEach(rhs.static_frames, [=](const std::pair<const FrameType&, const StaticFrameHandler&> kf)
	{
		static_frames[kf.first].CopyFrames(kf.second, frame_offset);
	});
}
