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

void Animator::SetFrame(FrameNumber frame_number)
{
	ForEach(key_frames, [=](const std::pair<const FrameType&, const KeyFrameHandler&> frame_handler)
	{
		const auto frame_value = frame_handler.second.GetFrame(frame_number);

		const auto& frame_type = frame_handler.first;

		bool result = false;

		switch (frame_type.tag)
		{
		case RLPA:
			result = ProcessRLPA(frame_type.index, frame_value);
			break;

		case RLTS:
			result = ProcessRLTS(frame_type.type, frame_type.index, frame_value);
			break;

		case RLVC:
			result = ProcessRLVC(frame_type.index, (u8)frame_value);
			break;

		case RLMC:
			result = ProcessRLMC(frame_type.index, (u8)frame_value);
			break;
		}

		//if (!result)
		//	std::cout << "unhandled frame (" << name << "): tag: " << (int)frame_type.tag
		//	<< " type: " << (int)frame_type.type
		//	<< " index: " << (int)frame_type.index
		//	<< '\n';
	});

	ForEach(static_frames, [=](const std::pair<const FrameType&, const StaticFrameHandler&> frame_handler)
	{
		auto const frame_data = frame_handler.second.GetFrame(frame_number);

		const auto& frame_type = frame_handler.first;

		bool result = false;

		switch (frame_type.tag)
		{
		case RLVI:
			result = ProcessRLVI(frame_data.data2);
			break;

		case RLTP:
			//result = ProcessRLTP(frame_data.data2);
			break;

		case RLIM:
			//result = ProcessRLIM(frame_data.data2);
			break;
		}

		//if (!result)
		//	std::cout << "unhandled frame (" << name << "): tag: " << (int)frame_type.tag
		//	<< " index: " << (int)frame_type.index
		//	<< '\n';
	});
}

void StaticFrameHandler::Load(std::istream& file, u16 count)
{
	while (count--)
	{
		FrameNumber frame;
		file >> BE >> frame;

		auto& pair = frames[frame];
		file >> BE >> pair.data1 >> pair.data2;
		file.seekg(2, std::ios::cur);	// these bytes important? :p

		//std::cout << "\t\t\t" "frame: " << frame << ' ' << pair.first << " " << pair.second << '\n';
	}
}

void KeyFrameHandler::Load(std::istream& file, u16 count)
{
	while (count--)
	{
		std::pair<FrameNumber, FrameData> pair;

		// read the frame number
		file >> BE >> pair.first;

		// read the value and slope
		file >> BE >> pair.second.value >> pair.second.slope;

		frames.insert(pair);

		//std::cout << "\t\t\t" "frame: " << frame << ' ' << frames[frame] << '\n';
	}
}

StaticFrameHandler::FrameData StaticFrameHandler::GetFrame(FrameNumber frame_number) const
{
	// assuming not empty, a safe assumption currently

	// find the current frame, or the one after it
	auto frame_it = frames.lower_bound(frame_number);

	// current frame is higher than any keyframe, use the last keyframe
	if (frames.end() == frame_it)
		--frame_it;

	// if this is after the current frame and not the first keyframe, use the previous one
	if (frame_number < frame_it->first && frames.begin() != frame_it)
		--frame_it;

	return frame_it->second;
}

float KeyFrameHandler::GetFrame(FrameNumber frame_number) const
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

	const float nf = next->first - prev->first;
	if (0 == nf)
	{
		// same frame numbers, just return the first's value
		return prev->second.value;
	}
	else
	{
		// different frames, blend them together
		// this is a "Cubic Hermite spline" apparently

		// clamp frame_number
		frame_number = std::max(prev->first, std::min(next->first, frame_number));
		
		const float t = (frame_number - prev->first) / nf;

		// old curve-less code
		//return prev->second.value + (next->second.value - prev->second.value) * t;
 
		// curvy code from marcan, :p
		return
			prev->second.slope * nf * (t + powf(t, 3) - 2 * powf(t, 2)) +
			next->second.slope * nf * (powf(t, 3) - powf(t, 2)) +
			prev->second.value * (1 + (2 * powf(t, 3) - 3 * powf(t, 2))) +
			next->second.value * (-2 * powf(t, 3) + 3 * powf(t, 2));
	}
}

void StaticFrameHandler::CopyFrames(const StaticFrameHandler& fh, FrameNumber frame_offset)
{
	ForEach(fh.frames, [=](const std::pair<const FrameNumber, const FrameData&> frame)
	{
		frames[frame.first + frame_offset] = frame.second;
	});
}

void KeyFrameHandler::CopyFrames(const KeyFrameHandler& fh, FrameNumber frame_offset)
{
	ForEach(fh.frames, [=](const std::pair<const FrameNumber, const FrameData&> frame)
	{
		frames.insert(std::make_pair(frame.first + frame_offset, frame.second));
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
