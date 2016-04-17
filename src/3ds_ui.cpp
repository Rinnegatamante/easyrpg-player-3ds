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

#include "system.h"
#ifdef _3DS

// Headers
#include "3ds_ui.h"
#include "color.h"
#include "graphics.h"
#include "keys.h"
#include "output.h"
#include "player.h"
#include "bitmap.h"
#include <iostream>
#include <sys/iosupport.h>

#include "../build/keyboard_bmp.h"

#include <3ds.h>
#include <sf2d.h>
#include <cstring>
#include <stdio.h>

#ifdef SUPPORT_AUDIO
#include "audio_3ds.h"
AudioInterface& CtrUi::GetAudio() {
	return *audio_;
}
#endif

static const devoptab_t dotab_null = {
	"null",
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

CtrUi::CtrUi(int width, int height) :
	BaseUi() {
	
	frame = 0;
	fullscreen = false;
	trigger_state = false;
	sf2d_init();
	current_display_mode.width = width;
	current_display_mode.height = height;
	current_display_mode.bpp = 32;
	const DynamicFormat format(
		32,
		0x000000FF,
		0x0000FF00,
		0x00FF0000,
		0xFF000000,
		PF::NoAlpha);
	Bitmap::SetFormat(Bitmap::ChooseFormat(format));
	main_surface = Bitmap::Create(width, height, true, 32);
	main_texture = sf2d_create_texture_mem_RGBA8(main_surface->pixels(),
	                                             main_surface->GetWidth(), main_surface->GetHeight(), 
	                                             TEXFMT_RGBA8, SF2D_PLACE_VRAM);
												 
	#ifdef SUPPORT_AUDIO
		audio_.reset(new CtrAudio());
	#endif
	
	#ifdef NO_DEBUG
	// Loading bottom screen keyboard
	u8* key_buffer = (u8*)&keyboard_bmp[0x36];
	u32 key_buffer_size = keyboard_bmp_size - 0x36;
	u8* key_buffer_rgba = (u8*)malloc((key_buffer_size/3)<<2);
	int z = 0;
	for(int i=0;i<key_buffer_size;i=i+3){
		key_buffer_rgba[z+2] = key_buffer[i];
		key_buffer_rgba[z+1] = key_buffer[i+1];
		key_buffer_rgba[z] = key_buffer[i+2];
		key_buffer_rgba[z+3] = 0xFF;
		z = z + 4;
	}
	keyboard_texture = sf2d_create_texture_mem_RGBA8(key_buffer_rgba,
	                                             320, 240, 
	                                             TEXFMT_RGBA8, SF2D_PLACE_RAM);
	free(key_buffer_rgba);
	
	// Disabling debug console
	devoptab_list[STD_OUT] = &dotab_null;
	devoptab_list[STD_ERR] = &dotab_null;
	consoleGetDefault()->frameBuffer = NULL;
	gfxSetScreenFormat(GFX_BOTTOM,GSP_BGR8_OES);
	gfxSetDoubleBuffering(GFX_BOTTOM,true);
	
	// Drawing keyboard once then unloading it
	for (int i=0;i<5;i++){ // If we don't print this a couple of time, image is corrupted
		sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
		sf2d_draw_texture(keyboard_texture, 0, 0);
		sf2d_end_frame();
		sf2d_swapbuffers();
	}
	sf2d_free_texture(keyboard_texture);
	#endif
	
}

CtrUi::~CtrUi() {
	sf2d_free_texture(main_texture);	
	sf2d_fini();
}

#define TICKS_PER_MSEC 268123.480
static inline double u64_to_double(u64 value) {
	return (((double)(u32)(value >> 32))*0x100000000ULL+(u32)value);
}

void CtrUi::Sleep(uint32_t time) {
	u64 nsecs = time*1000000;
	svcSleepThread(nsecs);
}

uint32_t CtrUi::GetTicks() const {
	double ticks = u64_to_double(svcGetSystemTick());
	u64 usecs = (u64)(ticks/TICKS_PER_MSEC);
	return usecs;
}
void CtrUi::BeginDisplayModeChange() {
	// no-op
}

void CtrUi::EndDisplayModeChange() {
	// no-op
}

void CtrUi::Resize(long /*width*/, long /*height*/) {
	// no-op
}

void CtrUi::ToggleFullscreen() {
	// no-op
}

void CtrUi::ToggleZoom() {
	// no-op
}

bool CtrUi::IsFullscreen() {
	return true;
}

void CtrUi::ProcessEvents() {
	
	hidScanInput();
	u32 input = hidKeysHeld();
	keys[Input::Keys::Z] = (input & KEY_A);
	keys[Input::Keys::X] = (input & KEY_B);
	keys[Input::Keys::N8] = (input & KEY_X);
	keys[Input::Keys::F12] = (input & KEY_SELECT);
	keys[Input::Keys::ESCAPE] = (input & KEY_START);
	keys[Input::Keys::RIGHT] = (input & KEY_DRIGHT);
	keys[Input::Keys::LEFT] = (input & KEY_DLEFT);
	keys[Input::Keys::UP] = (input & KEY_DUP);
	keys[Input::Keys::DOWN] = (input & KEY_DDOWN);
	keys[Input::Keys::F2] = (input & KEY_L);
	
	//Fullscreen mode support
	bool old_state = trigger_state;
	trigger_state = (input & KEY_R);
	if ((trigger_state != old_state) && trigger_state) fullscreen = !fullscreen;
	
	//CirclePad support
	circlePosition circlepad;
	hidCircleRead(&circlepad);
	
	if (circlepad.dy > 25) keys[Input::Keys::UP] = true;
	else if (circlepad.dy < -25) keys[Input::Keys::DOWN] = true;
	else if (circlepad.dx > 25) keys[Input::Keys::RIGHT] = true;
	else if (circlepad.dx < -25) keys[Input::Keys::LEFT] = true;
	
	#ifdef NO_DEBUG
	//Touchscreen support
	if (input & KEY_TOUCH){
		touchPosition pos;
		hidTouchRead(&pos);
		u8 row = pos.px>>6;
		u8 col = pos.py / 60;
		u32 keys_tbl[16] = {Input::Keys::N7, Input::Keys::N8, Input::Keys::N9,
						Input::Keys::DIVIDE, Input::Keys::N4, Input::Keys::N5,
						Input::Keys::N6, Input::Keys::MULTIPLY, Input::Keys::N1,
						Input::Keys::N2, Input::Keys::N3, Input::Keys::SUBTRACT,
						Input::Keys::N0, Input::Keys::N0, Input::Keys::PERIOD,
						Input::Keys::ADD
						};
		keys[keys_tbl[row + (col*4)]] = true;
	}else{
		keys[Input::Keys::N0] = false;
		keys[Input::Keys::N1] = false;
		keys[Input::Keys::N2] = false;
		keys[Input::Keys::N3] = false;
		keys[Input::Keys::N4] = false;
		keys[Input::Keys::N5] = false;
		keys[Input::Keys::N6] = false;
		keys[Input::Keys::N7] = false;
		keys[Input::Keys::N8] = false;
		keys[Input::Keys::N9] = false;
		keys[Input::Keys::DIVIDE] = false;
		keys[Input::Keys::MULTIPLY] = false;
		keys[Input::Keys::ADD] = false;
		keys[Input::Keys::SUBTRACT] = false;
		keys[Input::Keys::PERIOD] = false;
	}
	#endif
	
}

void CtrUi::UpdateDisplay() {
	main_texture->tiled = 0;
	sf2d_fill_texture_from_RGBA8(main_texture, main_surface->pixels(),
	                                             main_surface->GetWidth(), main_surface->GetHeight()
	                            );
	sf2d_start_frame(GFX_TOP, GFX_LEFT);
	if (!fullscreen) sf2d_draw_texture(main_texture, 40, 0);
	else sf2d_draw_texture_scale(main_texture, 0, 0, 1.25, 1.0);
	sf2d_end_frame();
	sf2d_swapbuffers();
}

void CtrUi::BeginScreenCapture() {
	CleanDisplay();
}

BitmapRef CtrUi::EndScreenCapture() {
	return Bitmap::Create(*main_surface, main_surface->GetRect());
}

void CtrUi::SetTitle(const std::string& /* title */) {
	// no-op
}

bool CtrUi::ShowCursor(bool /* flag */) {
	return true;
}

#endif
