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

#include <GL/glew.h>

#include "WiiBanner.h"

//#include <SFML/System.hpp>
#include <SFML/Window.hpp>

#define NO_CONSOLE 0

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
	sf::Window window(sf::VideoMode(608, 456, 32), "Wii Banner Player");
	
	std::string fname = "opening.bnr";	// just so i can test without setting params
	if (2 == argc)
		fname = argv[1];

	glewInit();

	WiiBanner banner(fname);

	window.Show(true);

	glViewport(0, 0, 608, 456);

	//glEnable(GL_DEPTH_TEST);
    //glDepthMask(GL_TRUE);

	//glEnable(GL_GENERATE_MIPMAP);

	// testing
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);

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
				glViewport(0, 0, ev.Size.Width, ev.Size.Height);
				break;

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

		if (banner_play)
			banner.AdvanceFrame();

		window.Display();

		// TODO: improve
		sf::Sleep(1.f / 60.f);
	}
}
