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

#include <iostream>
#include <fstream>

#include <iterator>
#include <algorithm>

#include <list>
#include <vector>
#include <sstream>
#include <string>

#include "WrapGx.h"

#include "WiiBanner.h"

#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>

#define NO_CONSOLE 0

void DrawBorder()
{
	static const int smooth = 20;
	static const double border = 1.10;
	static const float bottom = 0.4f;

	static const double pi = 3.141592653589793238462643;

	glLoadIdentity();
	glOrtho(-border, border, -border, border, -1, 1);

	glDisable(GL_BLEND);
	//glDisable(GL_TEXTURE);

	// sides
	glBegin(GL_QUADS);
	// left
	glVertex2f(-2.f, -2.f);
	glVertex2f(-1.f, -2.f);
	glVertex2f(-1.f, 2.f);
	glVertex2f(-2.f, 2.f);
	// top
	glVertex2f(-2.f, 2.f);
	glVertex2f(2.f, 2.f);
	glVertex2f(2.f, 1.f);
	glVertex2f(-2.f, 1.f);
	// right
	glVertex2f(1.f, -2.f);
	glVertex2f(2.f, -2.f);
	glVertex2f(2.f, 2.f);
	glVertex2f(1.f, 2.f);
	// bottom
	glVertex2f(-2.f, -1.f + bottom);
	glVertex2f(2.f, -1.f + bottom);
	glVertex2f(2.f, -2.f);
	glVertex2f(-2.f, -2.f);
	glEnd();

	// corners
	// TODO:
	//for (int c = 0; ; ++c)
	//{
	//	glBegin(GL_TRIANGLE_FAN);
	//	glVertex2f(2.f, 2.f);
	//	float angle = 0;
	//	for (unsigned int i = 0; i <= smooth; ++i, angle += pi / 2 / smooth)
	//		glVertex2f(std::sinf(angle), std::cosf(angle));
	//	glEnd();

	//	if (4 == c)
	//		break;
	//	glRotatef(90, 0, 0, 1);
	//}

	glEnable(GL_BLEND);
}

#if defined(_WIN32) && !defined(DEBUG) && NO_CONSOLE
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// hax
	int argc = 1 + (strlen(lpCmdLine) > 1);
	char* argv[2] = {"blah.exe", lpCmdLine};
#else
int main(int argc, char* argv[])
{
#endif	
	std::string fname = "opening.bnr";	// just so i can test without setting params
	if (2 == argc)
		fname = argv[1];

	// TODO: parse brlyt file separate from loading texture,
	// to get width/height without needing to have a window open first
	sf::Window window(sf::VideoMode(608, 456, 32), "Wii Banner Player");

	GX_Init(0, 0);

	WiiBanner banner(fname);

	window.SetSize(banner.GetWidth(), banner.GetHeight());

	//glViewport(0, 0, banner.GetWidth(), banner.GetHeight());

	glMatrixMode(GL_MODELVIEW);

	glEnable(GL_DEPTH_TEST);
    //glDepthMask(GL_TRUE);

	//glEnable(GL_COLOR_LOGIC_OP);
	//glLogicOp(GL_COPY);

	// testing
	glDepthFunc(GL_ALWAYS);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glBlendFunc(GL_ZERO, GL_ZERO);
	//glBlendFunc(GL_ZERO, GL_SRC_ALPHA);

	//glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);	// whats this even do? :p

	// test sfml-audio
	//sf::Sound sound;
	//sf::SoundBuffer buffer;
	//sf::Int16 samples[5000];
	//for (int i = 0; i != 5000; ++i)
	//	samples[i] = rand();
	//if (buffer.LoadFromSamples(samples, 5000, 2, 44100))
	//{
	//	sound.SetBuffer(buffer);
	//	sound.SetLoop(true);
	//	sound.SetPitch(0.5f);
	//	sound.SetVolume(20.f);
	//	sound.Play();
	//	sf::Sleep(1.0f);
	//	sound.Stop();
	//}

	//GLfloat max_anis;
	//glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_anis);
	//glTexParameterf(GL_TEXTURE_2D, GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, max_anis);

	bool banner_play = true;

	while (window.IsOpened())
	{
		sf::Event ev;
		while (window.GetEvent(ev))
		{
			switch (ev.Type)
			{
			case sf::Event::Closed:
				window.Close();
				break;

			case sf::Event::Resized:
				{
					const float
						banner_aspect = (float)banner.GetWidth() / (float)banner.GetHeight(),
						window_aspect = (float)ev.Size.Width / (float)ev.Size.Height;

					if (banner_aspect > window_aspect)
					{
						const GLsizei h = GLsizei(ev.Size.Width / banner_aspect);
						glViewport(0, (ev.Size.Height - h) / 2, ev.Size.Width, h);
					}
					else
					{
						const GLsizei w = GLsizei(ev.Size.Height * banner_aspect);
						glViewport((ev.Size.Width - w) / 2, 0, w, ev.Size.Height);
					}

					break;
				}

			case sf::Event::KeyPressed:
				switch (ev.Key.Code)
				{
					// pause resume playback
				case sf::Key::Space:
					banner_play ^= true;
					break;

					// previous frame
				case sf::Key::Left:
					banner.frame_current -= 1 + 4 * window.GetInput().IsKeyDown(sf::Key::LShift);
					break;

					// next frame
				case sf::Key::Right:
					banner.frame_current += 1 + 4 * window.GetInput().IsKeyDown(sf::Key::LShift);
					break;

					// restart playback
				case sf::Key::Back:
					banner.frame_current = 0;
					break;
				}
				break;
			}
		}

		window.SetActive();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0, 0, 0, 1);

		if (!banner_play)
			banner.SetFrame(banner.frame_current);

		banner.Render();

		//glColor4f(1.f, 1.f, 1.f, 1.f);
		//DrawBorder();

		if (banner_play)
			banner.AdvanceFrame();

		window.Display();

		// TODO: improve
		sf::Sleep(1.f / 60.f);
	}
}
