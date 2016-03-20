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

#ifndef _SPRITE_TIMER_H_
#define _SPRITE_TIMER_H_

// Headers
#include "sprite.h"

/**
 * Renders the timer on screen
 */
class Sprite_Timer : public Sprite {
public:
	/**
	 * Constructor.
	 *
	 * @param which Timer 1 or 2
	 */
	Sprite_Timer(int which);

	~Sprite_Timer();

	/**
	 * Updates sprite state.
	 */
	void Update();

protected:
	void CreateSprite();
	void Draw();

	int which;
	int counter;

	Rect digits[5];
};

#endif
