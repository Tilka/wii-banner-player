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

#include "Banner.h"
#include "Types.h"

#include "WrapGx.h"

#include <gl/glew.h>

#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>

#define NO_CONSOLE 0

void DrawBorder()
{
	glColor4f(0, 0, 0, 1.f);
	glUseProgram(0);

	glBegin(GL_QUADS);
	glVertex2f(-450.f, -112.f);
	glVertex2f(450.f, -112.f);
	glVertex2f(450.f, -400.f);
	glVertex2f(-450.f, -400.f);
	glEnd();

	// TODO: need to cover some of the top and sides
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

	sf::Window window(sf::VideoMode(608, 456, 32), "Wii Banner Player");

	static const float
		min_aspect = 4.f / 3,
		max_aspect = 16.f / 9;

	float render_aspect = min_aspect;

	GX_Init(0, 0);

	WiiBanner::Banner banner(fname);

	if (!(banner.GetBanner() && banner.GetIcon() && banner.GetSound()))
	{
		std::cout << "Failed to load banner.\n";
		std::cin.get();
		return 1;
	}

	banner.GetBanner()->SetLanguage("ENG");
	banner.GetIcon()->SetLanguage("ENG");	// TODO: do icons ever have groups?

	WiiBanner::Layout* layout = banner.GetBanner();

	glMatrixMode(GL_MODELVIEW);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);

	bool banner_play = true;
	banner.GetSound()->Play();

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
					const float window_aspect = (float)ev.Size.Width / (float)ev.Size.Height;
					
					render_aspect = Clamp(window_aspect, min_aspect, max_aspect);

					if (render_aspect > window_aspect)
					{
						const GLsizei h = GLsizei(ev.Size.Width / render_aspect);
						glViewport(0, (ev.Size.Height - h) / 2, ev.Size.Width, h);
					}
					else
					{
						const GLsizei w = GLsizei(ev.Size.Height * render_aspect);
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
					if (banner_play)
						banner.GetSound()->Play();
					else
						banner.GetSound()->Pause();
					break;

					// TODO make sound progress with frames, meh

					// previous frame
				case sf::Key::Left:
					layout->SetFrame(layout->GetFrame()
						- (1 + 4 * window.GetInput().IsKeyDown(sf::Key::LShift)));
					
					std::cout << "frame: " << layout->GetFrame() << '\n';
					break;

					// next frame
				case sf::Key::Right:
					layout->SetFrame(layout->GetFrame()
						+ (1 + 4 * window.GetInput().IsKeyDown(sf::Key::LShift)));

					std::cout << "frame: " << layout->GetFrame() << '\n';
					break;

					// restart playback
				case sf::Key::Back:
					layout->SetFrame(0);
					banner.GetSound()->Restart();
					if (banner_play)
						banner.GetSound()->Play();
					break;

					// exit app
				case sf::Key::Escape:
					window.Close();
					break;

					// change layout
				case sf::Key::Return:
					layout = (banner.GetBanner() == layout) ? banner.GetIcon() : banner.GetBanner();
					//window.SetSize((int)layout->GetWidth(), (int)layout->GetHeight());
					break;
				}
				break;
			}
		}

		window.SetActive();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0, 0, 0, 1.f);

		layout->Render(render_aspect);

		// TODO: reset any crazy blend modes and crap before drawing border
		if (banner.GetBanner() == layout)
			DrawBorder();

		window.Display();

		if (banner_play)
			layout->AdvanceFrame();

		// TODO: could be improved
		// limit fps
		static const int desired_fps = 60;
		static int frame_count = 0;
		static sf::Clock clock;
		static float sleep_time = 1.f / desired_fps;

		if (10 == frame_count)
		{
			sleep_time = (1.f / desired_fps) - (clock.GetElapsedTime() / frame_count - sleep_time);
			
			//std::cout << "sleep_time: " << (1.f / sleep_time) << '\n';

			frame_count = 0;
			clock.Reset();
		}
		else
			++frame_count;

		sf::Sleep(sleep_time);
	}
}
