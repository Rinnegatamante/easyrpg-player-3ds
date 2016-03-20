/*
 * This file is part of EasyRPG Player.
 *
 * EasyRPG Player is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
 */

// Headers
#include <iomanip>
#include <sstream>
#include "window_about.h"
#include "game_party.h"
#include "bitmap.h"
#include "font.h"

Window_About::Window_About(int ix, int iy, int iwidth, int iheight) :
	Window_Base(ix, iy, iwidth, iheight) {

	SetContents(Bitmap::Create(width - 16, height - 16));
}

void Window_About::Refresh() {
	std::vector<std::string> about_msg = {
		"EasyRPG is a RPG Maker 2000/2003",
		"interpreter.",
		"Licensed under the GPLv3",
		"",
		"Website: easy-rpg.org",
		"",
		"Contact us:",
		"Via Mail: easyrpg@easy-rpg.org",
		"Via Chat: #easyrpg @ freenode",
		"",
		"Bugs: github.com/EasyRPG/Player/issues",
		"Contribute: easy-rpg.org/contribute",
	};

	for (size_t i = 0; i < about_msg.size(); ++i) {
		contents->TextDraw(0, 2 + 14 * i, Font::ColorDefault, about_msg[i]);
	}
}
