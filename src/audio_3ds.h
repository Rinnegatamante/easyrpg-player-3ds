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
#include "audio.h"

#define SOUND_CHANNELS 22 // Number of available sounds channel

#include <map>
#include <3ds.h>
struct CtrAudio : public AudioInterface {
	CtrAudio();
	~CtrAudio();

	void BGM_Play(std::string const&, int, int, int);
	void BGM_Pause();
	void BGM_Resume();
	void BGM_Stop();
	bool BGM_PlayedOnce();
	unsigned BGM_GetTicks();
	void BGM_Fade(int);
	void BGM_Volume(int);
	void BGM_Pitch(int);
	void BGS_Play(std::string const&, int, int, int);
	void BGS_Pause();
	void BGS_Resume();
	void BGS_Stop();
	void BGS_Fade(int);
	void ME_Play(std::string const&, int, int, int);
	void ME_Stop();
	void ME_Fade(int /* fade */);
	void SE_Play(std::string const&, int, int);
	void SE_Stop();
	void Update();

	void BGM_OnPlayedOnce();
	int BGS_GetChannel() const;

private:
	u8* audiobuffers[SOUND_CHANNELS]; // We'll use last two available channels for BGM
	uint8_t num_channels = SOUND_CHANNELS;
	ndspWaveBuf dspSounds[SOUND_CHANNELS+1]; // We need one more waveBuf for BGM purposes
	int bgm_volume; // Stubbed
	bool (*isPlayingCallback)(int);
	void (*clearCallback)(int);
	u8 last_ch; // Used only with dsp::DSP

}; // class CtrAudio
