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

#ifndef _SPRITE_AIRSHIPSHADOW_H_
#define _SPRITE_AIRSHIPSHADOW_H_

// Headers
#include "sprite.h"
#include <string>

/**
 * Sprite for the shadow of the airship.
 *
 * Actually, the shadow is made of two 16x16 shadow sprites in the
 * System file drawn on top of each other. These are blit to an
 * intermediate bitmap and cached until the system graphic is changed.
 */
class Sprite_AirshipShadow : public Sprite {
public:
	Sprite_AirshipShadow();
	void Update();
	void RecreateShadow();
};

#endif
