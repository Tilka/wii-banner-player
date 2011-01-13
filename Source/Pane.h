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

#ifndef WII_BNR_PANE_H_
#define WII_BNR_PANE_H_

#include "Types.h"

#include <vector>

#include "Animator.h"
#include "Material.h"

// TODO: put all of this crap somewhere else
template <typename O, typename C, typename F>
void ReadOffsetList(std::istream& file, C count, std::streamoff origin, F func, std::streamoff pad = 0)
{
	std::streamoff next_offset = file.tellg();

	while (count--)
	{
		file.seekg(next_offset, std::ios::beg);

		O offset;
		file >> BE >> offset;
		file.seekg(pad, std::ios::cur);

		next_offset = file.tellg();

		file.seekg(origin + offset, std::ios::beg);
		func();
	}

	file.seekg(next_offset, std::ios::beg);
}

// multiply 2 color components
// assumes u8s, takes any type to avoid multiple conversions
template <typename C1, typename C2>
inline u8 MultiplyColors(C1 c1, C2 c2)
{
	return (u16)c1 * c2 / 0xff;
}

template <typename B, typename N>
void SetBit(B& flags, N bit, bool state)
{
	if (state)
		flags |= (1 << bit);
	else
		flags &= ~(1 << bit);
}

template <typename B, typename N>
bool GetBit(const B& flags, N bit)
{
	return !!(flags & (1 << bit));
}

namespace WiiBanner
{

struct Resources;

class Pane;
typedef std::vector<Pane*> PaneList;

class Pane : public Animator
{
public:
	typedef Animator Base;

	static const u32 BINARY_MAGIC = 'pan1';

	enum
	{
		NAME_LENGTH = 0x10,
		USER_DATA_LENGTH = 0x8
	};

	void Load(std::istream& file);
	virtual ~Pane();

	void Render(const Resources& resources, u8 parent_alpha, Vec2 adjust) const;
	void SetFrame(FrameNumber frame, u8 key_set);

	bool GetVisible() const { return GetBit(flags, FLAG_VISIBLE); }
	void SetVisible(bool visible) { SetBit(flags, FLAG_VISIBLE, visible); }

	// TODO: these ! are required?
	// TODO: better name?
	bool GetInfluencedAlpha() const { return !GetBit(flags, FLAG_INFLUENCED_ALPHA); }
	void SetInfluencedAlpha(bool influenced) { SetBit(flags, FLAG_INFLUENCED_ALPHA, !influenced); }

	// TODO: better name?
	bool GetPositionAdjust() const { return !GetBit(flags, FLAG_POSITION_ADJUST); }
	void SetPositionAdjust(bool adjust) { SetBit(flags, FLAG_POSITION_ADJUST, !adjust); }

	bool GetHide() const { return hide; }
	void SetHide(bool _hide) { hide = _hide; }

	u8 GetOriginX() const { return origin % 3; }
	u8 GetOriginY() const { return 2 - origin / 3; }

	float GetWidth() const { return width; }
	float GetHeight() const { return height; }

	u8 GetAlpha() const { return alpha; }
	void SetAlpha(u8 _alpha) { alpha = _alpha; }

	const Vec3& GetTranslate() const { return translate; }
	void SetTranslate(const Vec3& _translate) { translate = _translate; }

	const Vec3& GetRotate() const { return rotate; }
	void SetRotate(const Vec3& _rotate) { rotate = _rotate; }

	const Vec2& GetScale() const { return scale; }
	void SetScale(const Vec2& _scale) { scale = _scale; }

	Pane* FindPane(const std::string& name);	// recursive

	PaneList panes;

protected:
	void ProcessHermiteKey(const KeyType& type, float value);
	void ProcessStepKey(const KeyType& type, StepKeyHandler::KeyData data);

private:
	virtual void Draw(const Resources& resources, u8 render_alpha) const {};

	enum : u8
	{
		FLAG_VISIBLE,
		FLAG_INFLUENCED_ALPHA,
		FLAG_POSITION_ADJUST
	};

	u8 origin;
	u8 flags;
	u8 alpha;
	bool hide;	// used by the groups

	Vec3 translate, rotate;
	Vec2 scale;
	float width, height;
};

// apparently Bounding is just a regular pane
class Bounding : public Pane
{
public:
	static const u32 BINARY_MAGIC = 'bnd1';
};

// used by Picture and Window
// TODO: put elsewhere I think
class Quad : public Pane
{
public:
	typedef Pane Base;

	void Load(std::istream& file);

protected:
	Quad() {}

	void ProcessHermiteKey(const KeyType& type, float value);

	void Draw(const Resources& resources, u8 render_alpha) const;

private:
	struct TexCoords
	{
		struct TexCoord
		{
			float s, t;

		} coords[4];
	};
	std::vector<TexCoords> tex_coords;

	u16 material_index;

	GXColor vertex_colors[4];
};

}

#endif
