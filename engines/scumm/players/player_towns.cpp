/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include "scumm/sound.h"
#include "scumm/players/player_towns.h"

namespace Scumm {

Player_Towns::Player_Towns(ScummEngine *vm, bool isVersion2) : _vm(vm), _v2(isVersion2), _numSoundMax(isVersion2 ? 256 : 200) {
	memset(_pcmCurrentSound, 0, sizeof(_pcmCurrentSound));
}

void Player_Towns::setSfxVolume(int vol) {
	if (!_intf)
		return;
	_intf->setSoundEffectVolume(vol);
}

int Player_Towns::getSoundStatus(int sound) const {
	if (!_intf)
		return 0;
	for (int i = 1; i < 9; i++) {
		if (_pcmCurrentSound[i].index == sound)
			return _intf->callback(40, 0x3f + i) ? 1 : 0;
	}
	return 0;
}

void syncWithSerializer(Common::Serializer &s, Player_Towns::PcmCurrentSound &pcs) {
	s.syncAsSint16LE(pcs.index, VER(81));
	s.syncAsSint16LE(pcs.chan, VER(81));
	s.syncAsByte(pcs.note, VER(81));
	s.syncAsByte(pcs.velo, VER(81));
	s.syncAsByte(pcs.pan, VER(81));
	s.syncAsByte(pcs.paused, VER(81));
	s.syncAsByte(pcs.looping, VER(81));
	s.syncAsUint32LE(pcs.priority, VER(81));
}

void Player_Towns::saveLoadWithSerializer(Common::Serializer &s) {
	for (int i = 1; i < 9; i++) {
		if (!_pcmCurrentSound[i].index)
			continue;

		if (_intf->callback(40, i + 0x3f))
			continue;

		_intf->callback(39, i + 0x3f);

		_pcmCurrentSound[i].index = 0;
	}

	s.syncArray(_pcmCurrentSound, 9, syncWithSerializer);
}

void Player_Towns::restoreAfterLoad() {
	Common::Array<uint16> restoredSounds;

	for (int i = 1; i < 9; i++) {
		if (!_pcmCurrentSound[i].index || _pcmCurrentSound[i].index == 0xffff)
			continue;

		// Don't restart multichannel sounds more than once
		if (Common::find(restoredSounds.begin(), restoredSounds.end(), _pcmCurrentSound[i].index) != restoredSounds.end())
			continue;

		if (!_v2)
			restoredSounds.push_back(_pcmCurrentSound[i].index);

		uint8 *ptr = _vm->getResourceAddress(rtSound, _pcmCurrentSound[i].index);
		if (!ptr)
			continue;

		if (_vm->_game.version != 3)
			ptr += 2;

		if (ptr[13])
			continue;

		playPcmTrack(_pcmCurrentSound[i].index, ptr + 6, _pcmCurrentSound[i].velo, _pcmCurrentSound[i].pan, _pcmCurrentSound[i].note, _pcmCurrentSound[i].priority);
	}
}

void Player_Towns::playPcmTrack(int sound, const uint8 *data, int velo, int pan, int note, int priority) {
	if (!_intf)
		return;

	const uint8 *sfxData = data + 16;

	int numChan = _v2 ? 1 : data[14];
	for (int i = 0; i < numChan; i++) {
		int chan = allocatePcmChannel(sound, i, priority);
		if (!chan)
			return;

		_intf->callback(70, _unkFlags);
		_intf->callback(3, chan + 0x3f, pan);
		_intf->callback(37, chan + 0x3f, note, velo, sfxData);

		_pcmCurrentSound[chan].note = note;
		_pcmCurrentSound[chan].velo = velo;
		_pcmCurrentSound[chan].pan = pan;
		_pcmCurrentSound[chan].paused = 0;
		_pcmCurrentSound[chan].looping = READ_LE_UINT32(&sfxData[20]) ? 1 : 0;

		sfxData += (READ_LE_UINT32(&sfxData[12]) + 32);
	}
}

void Player_Towns::stopPcmTrack(int sound) {
	if (!_intf)
		return;

	for (int i = 1; i < 9; i++) {
		if (sound == _pcmCurrentSound[i].index || !sound) {
			_intf->callback(39, i + 0x3f);
			_pcmCurrentSound[i].index = 0;
		}
	}
}

int Player_Towns::allocatePcmChannel(int sound, int sfxChanRelIndex, uint32 priority) {
	if (!_intf)
		return 0;

	int chan = 0;

	if (_v2 && priority > 255) {
		chan = 8;
		if (_intf->callback(40, 0x47))
			_intf->callback(39, 0x47);
	} else {
		for (int i = 8; i; i--) {
			if (!_pcmCurrentSound[i].index) {
				chan = i;
				continue;
			}

			if (_intf->callback(40, i + 0x3f))
				continue;

			chan = i;
			if (_pcmCurrentSound[chan].index == 0xffff)
				_intf->callback(39, chan + 0x3f);
			else
				_vm->_sound->stopSound(_pcmCurrentSound[chan].index);
		}

		if (!chan) {
			for (int i = 1; i < 9; i++) {
				if (priority >= _pcmCurrentSound[i].priority)
					chan = i;
			}
			if (_pcmCurrentSound[chan].index == 0xffff)
				_intf->callback(39, chan + 0x3f);
			else
				_vm->_sound->stopSound(_pcmCurrentSound[chan].index);
		}
	}

	if (chan) {
		_pcmCurrentSound[chan].index = sound;
		_pcmCurrentSound[chan].chan = sfxChanRelIndex;
		_pcmCurrentSound[chan].priority = priority;
	}

	return chan;
}

Player_Towns_v1::Player_Towns_v1(ScummEngine *vm, Audio::Mixer *mixer) : Player_Towns(vm, false) {
	if (_vm->_game.version == 3) {
		_soundOverride = new SoundOvrParameters[_numSoundMax]();
	}

	_player = new EuphonyPlayer(mixer);
	_intf = new TownsAudioInterface(mixer, nullptr);
}

Player_Towns_v1::~Player_Towns_v1() {
	delete _intf;
	delete _player;
	delete[] _soundOverride;
}

bool Player_Towns_v1::init() {
	if (!_player)
		return false;

	if (!_player->init())
		return false;

	_player->driver()->reserveSoundEffectChannels(8);

	// Treat all 6 fm channels and all 8 pcm channels as sound effect channels
	// since music seems to exist as CD audio only in the games which use this
	// MusicEngine implementation.
	_intf->setSoundEffectChanMask(-1);

	setVolumeCD(255, 255);

	return true;
}

void Player_Towns_v1::setMusicVolume(int vol) {
	_player->driver()->setMusicVolume(vol);
}

void Player_Towns_v1::startSound(int sound) {
	uint8 *ptr = _vm->getResourceAddress(rtSound, sound);
	assert(ptr);

	if (_vm->_game.version != 3)
		ptr += 2;

	int type = ptr[13];

	if (type == 0) {
		uint8 velocity = 0;
		uint8 note = 0;

		if (_vm->_game.version == 3) {
			velocity = (_soundOverride[sound].vLeft + _soundOverride[sound].vRight);
			note = _soundOverride[sound].note;
		}

		velocity = velocity ? velocity >> 2 : ptr[14] >> 1;
		uint16 len = READ_LE_UINT16(ptr) + 2;
		playPcmTrack(sound, ptr + 6, velocity, 64, note ? note : (len > 50 ? ptr[50] : 60), READ_LE_UINT16(ptr + 10));
	} else if (type == 1 || (_vm->_game.id == GID_INDY3 && sound == 40 && _vm->enhancementEnabled(kEnhAudioChanges))) {
		// WORKAROUND: Indy 3 FMTOWNS: No/distorted music in Venice
		// The Venice music does not exist as CD audio and the original doesn't feature music
		// in this scene. It does, however, exist as Euphony track albeit with an odd sound
		// type (255 instead of 1).
		// It doesn't sound great but we'll enable it to have music at all in this scene.
		// See Trac#1873 and Trac#10561.
		playEuphonyTrack(sound, ptr + 6);

	} else if (type == 2) {
		playCdaTrack(sound, ptr + 6);
	}

	if (_vm->_game.version == 3)
		_soundOverride[sound].vLeft = _soundOverride[sound].vRight = _soundOverride[sound].note = 0;
}

void Player_Towns_v1::stopSound(int sound) {
	if (sound == 0 || sound == _cdaCurrentSound) {
		_cdaCurrentSound = 0;
		_vm->_sound->stopCD();
	}

	if (sound != 0 && sound == _eupCurrentSound) {
		_eupCurrentSound = 0;
		_eupLooping = false;
		_player->stop();
	}

	stopPcmTrack(sound);
}

void Player_Towns_v1::stopAllSounds() {
	_cdaCurrentSound = 0;
	_vm->_sound->stopCD();

	_eupCurrentSound = 0;
	_eupLooping = false;
	_player->stop();

	stopPcmTrack(0);
}

int Player_Towns_v1::getSoundStatus(int sound) const {
	if (sound == _cdaCurrentSound)
		return _vm->_sound->pollCD();
	if (sound == _eupCurrentSound)
		return _player->isPlaying() ? 1 : 0;
	return Player_Towns::getSoundStatus(sound);
}

int32 Player_Towns_v1::doCommand(int numargs, int args[]) {
	int32 res = 0;

	switch (args[0]) {
	case 2:
		_player->driver()->cdaToggle(0);
		break;

	case 3:
		restartLoopingSounds();
		break;

	case 8:
		startSound(args[1]);
		break;

	case 9:
		_vm->_sound->stopSound(args[1]);
		break;

	case 11:
		stopPcmTrack(0);
		break;

	case 14:
		startSoundEx(args[1], args[2], args[3], args[4]);
		break;

	case 15:
		stopSoundSuspendLooping(args[1]);
		break;

	default:
		warning("Player_Towns_v1::doCommand: Unknown command %d", args[0]);
		break;
	}

	return res;
}

void Player_Towns_v1::setVolumeCD(int left, int right) {
	_cdaVolLeft = left & 0xff;
	_cdaVolRight = right & 0xff;
	_player->driver()->setOutputVolume(1, left >> 1, right >> 1);
}

void Player_Towns_v1::setSoundVolume(int sound, int left, int right) {
	if (_soundOverride && sound > 0 && sound < _numSoundMax) {
		_soundOverride[sound].vLeft = left;
		_soundOverride[sound].vRight = right;
	}
}

void Player_Towns_v1::setSoundNote(int sound, int note) {
	if (_soundOverride && sound > 0 && sound < _numSoundMax)
		_soundOverride[sound].note = note;
}

void Player_Towns_v1::saveLoadWithSerializer(Common::Serializer &s) {
	_cdaCurrentSoundTemp = (_vm->_sound->pollCD() && _cdaNumLoops > 1) ? _cdaCurrentSound & 0xff : 0;
	_cdaNumLoopsTemp = _cdaNumLoops & 0xff;

	s.syncAsByte(_cdaCurrentSoundTemp, VER(81));
	s.syncAsByte(_cdaNumLoopsTemp, VER(81));
	s.syncAsByte(_cdaVolLeft, VER(81));
	s.syncAsByte(_cdaVolRight, VER(81));

	if (!_eupLooping && !_player->isPlaying())
		_eupCurrentSound = 0;

	s.syncAsByte(_eupCurrentSound, VER(81));
	s.syncAsByte(_eupLooping, VER(81));
	s.syncAsByte(_eupVolLeft, VER(81));
	s.syncAsByte(_eupVolRight, VER(81));

	Player_Towns::saveLoadWithSerializer(s);
}

void Player_Towns_v1::restoreAfterLoad() {
	setVolumeCD(_cdaVolLeft, _cdaVolRight);

	if (_cdaCurrentSoundTemp) {
		uint8 *ptr = _vm->getResourceAddress(rtSound, _cdaCurrentSoundTemp) + 6;
		if (_vm->_game.version != 3)
			ptr += 2;

		if (ptr[7] == 2) {
			playCdaTrack(_cdaCurrentSoundTemp, ptr, true);
			_cdaCurrentSound = _cdaCurrentSoundTemp;
			_cdaNumLoops = _cdaNumLoopsTemp;
		}
	}

	if (_eupCurrentSound) {
		uint8 *ptr = _vm->getResourceAddress(rtSound, _eupCurrentSound) + 6;
		if (_vm->_game.version != 3)
			ptr += 2;

		// WORKAROUND for bug #1873 INDY3 FMTOWNS: Music in Venice is distorted
		// The resource for sound 40 accidentally sets the sound type to 255 instead of 1.
		if (ptr[7] == 1 || (_vm->_game.id == GID_INDY3 && _eupCurrentSound == 40)) {
			setSoundVolume(_eupCurrentSound, _eupVolLeft, _eupVolRight);
			playEuphonyTrack(_eupCurrentSound, ptr);
		}
	}

	Player_Towns::restoreAfterLoad();
}

void Player_Towns_v1::restartLoopingSounds() {
	if (_cdaNumLoops && !_cdaForceRestart)
		_cdaForceRestart = 1;

	for (int i = 1; i < 9; i++) {
		if (!_pcmCurrentSound[i].paused)
			continue;

		_pcmCurrentSound[i].paused = 0;

		uint8 *ptr = _vm->getResourceAddress(rtSound, _pcmCurrentSound[i].index);
		if (!ptr)
			continue;
		ptr += 24;

		int c = 1;
		while (_pcmCurrentSound[i].chan != c) {
			ptr = ptr + READ_LE_UINT32(&ptr[12]) + 32;
			c++;
		}

		_player->driver()->playSoundEffect(i + 0x3f, _pcmCurrentSound[i].note, _pcmCurrentSound[i].velo, ptr);
	}

	_player->driver()->cdaToggle(1);
}

void Player_Towns_v1::startSoundEx(int sound, int velo, int pan, int note) {
	uint8 *ptr = _vm->getResourceAddress(rtSound, sound) + 2;

	if (pan > 99)
		pan = 99;

	velo = velo ? (velo * ptr[14] + 50) / 100 : ptr[14];
	velo = CLIP(velo, 1, 255);
	uint16 pri = READ_LE_UINT16(ptr + 10);

	if (ptr[13] == 0) {
		velo >>= 1;

		if (!velo)
			velo = 1;

		pan = pan ? (((pan << 7) - pan) + 50) / 100 : 64;

		playPcmTrack(sound, ptr + 6, velo, pan, note ? note : ptr[50], pri);

	} else if (ptr[13] == 2) {
		int volLeft = velo;
		int volRight = velo;

		if (pan < 50)
			volRight = ((pan * 2 + 1) * velo + 50) / 100;
		else if (pan > 50)
			volLeft = (((99 - pan) * 2 + 1) * velo + 50) / 100;

		setVolumeCD(volLeft, volRight);

		if (!_cdaForceRestart && sound == _cdaCurrentSound)
			return;

		playCdaTrack(sound, ptr + 6, true);
	}
}

void Player_Towns_v1::stopSoundSuspendLooping(int sound) {
	if (!sound) {
		return;
	} else if (sound == _cdaCurrentSound) {
		if (_cdaNumLoops && _cdaForceRestart)
			_cdaForceRestart = 1;
	} else {
		for (int i = 1; i < 9; i++) {
			if (sound == _pcmCurrentSound[i].index) {
				if (!_player->driver()->soundEffectIsPlaying(i + 0x3f))
					continue;
				_player->driver()->stopSoundEffect(i + 0x3f);
				if (_pcmCurrentSound[i].looping)
					_pcmCurrentSound[i].paused = 1;
				else
					_pcmCurrentSound[i].index = 0;
			}
		}
	}
}

void Player_Towns_v1::playEuphonyTrack(int sound, const uint8 *data) {
	const uint8 *pos = data + 16;
	const uint8 *src = pos + data[14] * 48;
	const uint8 *trackData = src + 150;

	for (int i = 0; i < 32; i++)
		_player->configPart_enable(i, *src++);
	for (int i = 0; i < 32; i++)
		_player->configPart_setType(i, 0xff);
	for (int i = 0; i < 32; i++)
		_player->configPart_remap(i, *src++);
	for (int i = 0; i < 32; i++)
		_player->configPart_adjustVolume(i, *src++);
	for (int i = 0; i < 32; i++)
		_player->configPart_setTranspose(i, *src++);

	src += 8;
	for (int i = 0; i < 6; i++)
		_player->driver()->assignPartToChannel(i, *src++);

	for (int i = 0; i < data[14]; i++) {
		_player->driver()->loadInstrument(i, i, pos + i * 48);
		_player->driver()->setInstrument(i, i);
	}

	_eupVolLeft = _soundOverride[sound].vLeft;
	_eupVolRight = _soundOverride[sound].vRight;
	int lvl = _soundOverride[sound].vLeft + _soundOverride[sound].vRight;
	if (!lvl)
		lvl = data[8] + data[9];
	lvl >>= 2;

	for (int i = 0; i < 6; i++)
		_player->driver()->channelVolume(i, lvl);

	uint32 trackSize = READ_LE_UINT32(src);
	src += 4;
	uint8 startTick = *src++;

	_player->setTempo(*src++);
	_player->startTrack(trackData, trackSize, startTick);

	_eupLooping = (*src != 1) ? 1 : 0;
	_player->setLoopStatus(_eupLooping != 0);
	_player->resume();
	_eupCurrentSound = sound;
}

void Player_Towns_v1::playCdaTrack(int sound, const uint8 *data, bool skipTrackVelo) {
	const uint8 *ptr = data;

	if (!sound)
		return;

	if (!skipTrackVelo) {
		if (_vm->_game.version == 3) {
			if (_soundOverride[sound].vLeft + _soundOverride[sound].vRight)
				setVolumeCD(_soundOverride[sound].vLeft, _soundOverride[sound].vRight);
			else
				setVolumeCD(ptr[8], ptr[9]);
		} else {
			setVolumeCD(ptr[8], ptr[9]);
		}
	}

	if (sound == _cdaCurrentSound && _vm->_sound->pollCD() == 1)
		return;

	ptr += 16;

	int track = ptr[0];
	_cdaNumLoops = ptr[1];
	int start = (ptr[2] * 60 + ptr[3]) * 75 + ptr[4];
	int end = (ptr[5] * 60 + ptr[6]) * 75 + ptr[7];

	_vm->_sound->playCDTrack(track, _cdaNumLoops == 0xff ? -1 : _cdaNumLoops, start, end <= start ? 0 : end - start);
	_cdaForceRestart = 0;
	_cdaCurrentSound = sound;
}

Player_Towns_v2::Player_Towns_v2(ScummEngine *vm, Audio::Mixer *mixer, IMuse *imuse, bool disposeIMuse)
	: Player_Towns(vm, true), _imuse(imuse), _imuseDispose(disposeIMuse) {
	_soundOverride = new SoundOvrParameters[_numSoundMax]();
	_intf = new TownsAudioInterface(mixer, nullptr, true);
}

Player_Towns_v2::~Player_Towns_v2() {
	delete _intf;
	_intf = nullptr;

	if (_imuseDispose)
		delete _imuse;

	delete[] _sblData;
	delete[] _soundOverride;
}

bool Player_Towns_v2::init() {
	if (!_intf)
		return false;

	if (!_intf->init())
		return false;

	_intf->callback(33, 8);
	_intf->setSoundEffectChanMask(~0x3f);

	return true;
}

void Player_Towns_v2::setMusicVolume(int vol) {
	_imuse->setMusicVolume(vol);
}

int Player_Towns_v2::getSoundStatus(int sound) const {
	if (_soundOverride[sound].type == 7)
		return Player_Towns::getSoundStatus(sound);
	return _imuse->getSoundStatus(sound);
}

void Player_Towns_v2::startSound(int sound) {
	uint8 *ptr = _vm->getResourceAddress(rtSound, sound);
	assert(ptr);

	if (READ_BE_UINT32(ptr) == MKTAG('T','O','W','S')) {
		_soundOverride[sound].type = 7;
		uint8 velo = _soundOverride[sound].velo ? _soundOverride[sound].velo - 1: (ptr[10] + ptr[11] + 1) >> 1;
		uint8 pan = _soundOverride[sound].pan ? _soundOverride[sound].pan - 1 : 64;
		uint8 pri = ptr[9];
		_soundOverride[sound].velo = _soundOverride[sound].pan = 0;
		playPcmTrack(sound, ptr + 8, velo, pan, ptr[52], pri);

	} else if (READ_BE_UINT32(ptr) == MKTAG('S','B','L',' ')) {
		_soundOverride[sound].type = 5;
		playVocTrack(ptr + 27);

	} else {
		_soundOverride[sound].type = 3;
		_imuse->startSound(sound);
	}
}

void Player_Towns_v2::stopSound(int sound) {
	if (_soundOverride[sound].type == 7) {
		stopPcmTrack(sound);
	} else {
		_imuse->stopSound(sound);
	}
}

void Player_Towns_v2::stopAllSounds() {
	stopPcmTrack(0);
	_imuse->stopAllSounds();
}

int32 Player_Towns_v2::doCommand(int numargs, int args[]) {
	int32 res = -1;
	uint8 *ptr = nullptr;

	switch (args[0]) {
	case 8:
		startSound(args[1]);
		res = 0;
		break;

	case 9:
	case 15:
		stopSound(args[1]);
		res = 0;
		break;

	case 11:
		stopPcmTrack(0);
		break;

	case 13:
		res = getSoundStatus(args[1]);
		break;

	case 258:
		if (_soundOverride[args[1]].type == 0) {
			ptr = _vm->getResourceAddress(rtSound, args[1]);
			if (READ_BE_UINT32(ptr) == MKTAG('T','O','W','S'))
				_soundOverride[args[1]].type = 7;
		}
		if (_soundOverride[args[1]].type == 7)	{
			_soundOverride[args[1]].velo = args[2] + 1;
			res = 0;
		}
		break;

	case 259:
		if (_soundOverride[args[1]].type == 0) {
			ptr = _vm->getResourceAddress(rtSound, args[1]);
			if (READ_BE_UINT32(ptr) == MKTAG('T','O','W','S'))
				_soundOverride[args[1]].type = 7;
		}
		if (_soundOverride[args[1]].type == 7)	{
			_soundOverride[args[1]].pan = 64 - CLIP<int>(args[2], -63, 63);
			res = 0;
		}
		break;

	default:
		break;
	}

	if (res == -1)
		return _imuse->doCommand(numargs, args);

	return res;
}

void Player_Towns_v2::saveLoadWithSerializer(Common::Serializer &s) {
	if (s.getVersion() >= VER(83))
		Player_Towns::saveLoadWithSerializer(s);
}

void Player_Towns_v2::playVocTrack(const uint8 *data) {
	static const uint8 header[] = {
		0x54, 0x61, 0x6C, 0x6B, 0x69, 0x65, 0x20, 0x20,
		0x78, 0x56, 0x34, 0x12, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,
		0x36, 0x04, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x00
	};

	uint32 len = (READ_LE_UINT32(data) >> 8) - 2;

	int chan = allocatePcmChannel(0xffff, 0, 0x1000);
	if (!chan)
		return;

	delete[] _sblData;
	_sblData = new uint8[len + 32];

	memcpy(_sblData, header, 32);
	WRITE_LE_UINT32(_sblData + 12, len);

	const uint8 *src = data + 6;
	uint8 *dst = _sblData + 32;
	for (uint32 i = 0; i < len; i++)
		*dst++ = *src & 0x80 ? (*src++ & 0x7f) : -*src++;

	_intf->callback(37, 0x3f + chan, 60, 127, _sblData);
	_pcmCurrentSound[chan].paused = 0;
}

} // End of namespace Scumm
