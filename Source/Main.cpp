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

#include "WiiBanner.h"

#include <SFML/Window.hpp>

#if defined(_WIN32) && !defined(DEBUG)
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

	WiiBanner banner(fname);

	// print the pane layout
	ForEach(banner.panes, [&](const Pane* pane)
	{
		pane->Print(0);
	});

	window.Show(true);

	glViewport(0, 0, 608, 456);

	//glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);

	GLuint gltex;
	glGenTextures(1, &gltex);

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
			}
		}

		window.SetActive();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0, 0, 0, 1);

		banner.Render();
		banner.AdvanceFrame();

		window.Display();

		// TODO: improve
		sf::Sleep(1.f / 60.f);
	}
}
