/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef PLATFORM_3DS_H
#define PLATFORM_3DS_H

#define FORBIDDEN_SYMBOL_EXCEPTION_time_h

#include "backends/mutex/mutex.h"
#include "backends/base-backend.h"
#include "graphics/palette.h"
#include "base/main.h"
#include "audio/mixer_intern.h"
#include "backends/graphics/graphics.h"
#include "backends/platform/3ds/sprite.h"
#include "common/rect.h"
#include "common/queue.h"

#define TICKS_PER_MSEC 268123

namespace _3DS {

enum MagnifyMode {
	MODE_MAGON,
	MODE_MAGOFF,
};

enum InputMode {
	MODE_HOVER,
	MODE_DRAG,
};

enum GraphicsModeID {
	RGBA8,
	RGB565,
	RGB8,
	RGB555,
	RGB5A1,
	CLUT8
};

// Graphics transaction states
enum TransactionState {
	kTransactionNone = 0,
	kTransactionActive = 1,
	kTransactionRollback = 2
};


// Details for a graphics transaction
struct TransactionDetails {
	bool formatChanged, modeChanged;

	TransactionDetails() {
		formatChanged = false;
		modeChanged = false;
	}
};

typedef struct GfxMode3DS {
	Graphics::PixelFormat svmPF;
	GSPGPU_FramebufferFormats screenFormat;
	GPU_COLORBUF bufferFormat;
	GPU_TEXCOLOR textureFormat;
	uint32 displayTransferFlags;
	uint32 textureTransferFlags;
} GfxMode3DS;

// Graphics configuration properties
struct GfxState {
	bool setup;
	GraphicsModeID gfxModeID;
	GfxMode3DS *gfxMode;
	MagnifyMode magMode;

/*	Graphics::PixelFormat svmPF;
	GSPGPU_FramebufferFormats screenFormat;
	GPU_COLORBUF bufferFormat;
	GPU_TEXCOLOR textureFormat;
	uint32 displayTransferFlags;
	uint32 textureTransferFlags;*/

	GfxState() {
		setup = false;
		gfxModeID = CLUT8;
		magMode = MODE_MAGOFF;
	}
};

// Game buffer configuration properties
struct GameState {
	u16 width, height;
	u16 topX, topY;
	u16 bottomX, bottomY;
	Graphics::PixelFormat pixfmt;

	GameState() {
		width = 320;
		height = 240;
		topX = bottomX = 0;
		topY = bottomY = 0;
		pixfmt = Graphics::PixelFormat::createFormatCLUT8();
	}
};

// Cursor configuration state
struct CursorState {
	Common::Rect area;
	float coordXScreen, coordYScreen;
	float coordXOverlay, coordYOverlay;
	float deltaX, deltaY;
	int hotspotX, hotspotY;
	Graphics::PixelFormat pixfmt;
	uint32 keyColor;
	bool visible;
	bool scalable;
	bool paletteEnabled;

	CursorState() {
		area = Common::Rect(0, 0, 0, 0);
		coordXScreen = coordXOverlay = 0;
		coordYScreen = coordYOverlay = 0;
		deltaX = deltaY = 0;
		hotspotX = hotspotY = 0;
		pixfmt = Graphics::PixelFormat::createFormatCLUT8();
		keyColor = 0;
		visible = scalable = paletteEnabled = false;
	}
};

// Overlay configuration state
struct OverlayState {
	

	OverlayState() {
		
	}
};

// Magnify configuration state
struct MagState {
	u16 x, y;
	u16 width, height;
	u16 centerX() { return width / 2; }
	u16 centerY() { return height / 2; }

	MagState() {
		x = y = 0;
		width = 400;
		height = 240;
	}
};


class OSystem_3DS : public EventsBaseBackend, public PaletteManager {
public:
	OSystem_3DS();
	virtual ~OSystem_3DS();

	volatile bool exiting;
	volatile bool sleeping;

	virtual void initBackend();

	virtual bool hasFeature(OSystem::Feature f);
	virtual void setFeatureState(OSystem::Feature f, bool enable);
	virtual bool getFeatureState(OSystem::Feature f);

	virtual bool pollEvent(Common::Event &event);

	virtual uint32 getMillis(bool skipRecord = false);
	virtual void delayMillis(uint msecs);
	virtual void getTimeAndDate(TimeDate &t) const;

	virtual MutexRef createMutex();
	virtual void lockMutex(MutexRef mutex);
	virtual void unlockMutex(MutexRef mutex);
	virtual void deleteMutex(MutexRef mutex);

	virtual void logMessage(LogMessageType::Type type, const char *message);

	virtual Audio::Mixer *getMixer();
	virtual PaletteManager *getPaletteManager() { return this; }
	virtual Common::String getSystemLanguage() const;
	virtual void fatalError();
	virtual void quit();

	virtual Common::String getDefaultConfigFileName();
	void addSysArchivesToSearchSet(Common::SearchSet &s, int priority) override;

	// Graphics
	inline Graphics::PixelFormat getScreenFormat() const { return _gameState.pixfmt; }
	virtual Common::List<Graphics::PixelFormat> getSupportedFormats() const;
	void initSize(uint width, uint height,
	              const Graphics::PixelFormat *format = NULL);
	virtual int getScreenChangeID() const { return _screenChangeId; };
	GraphicsModeID chooseMode(Graphics::PixelFormat *format);
	bool setGraphicsMode(GraphicsModeID modeID);

	void beginGFXTransaction();
	OSystem::TransactionError endGFXTransaction();
	int16 getHeight(){ return _gameState.height; }
	int16 getWidth(){ return _gameState.width; }
	float getScaleRatio() const;
	void setPalette(const byte *colors, uint start, uint num);
	void grabPalette(byte *colors, uint start, uint num) const;
	void copyRectToScreen(const void *buf, int pitch, int x, int y, int w,
	                      int h);
	Graphics::Surface *lockScreen();
	void unlockScreen();
	void updateScreen();
	void setShakePos(int shakeXOffset, int shakeYOffset);
	void setFocusRectangle(const Common::Rect &rect);
	void clearFocusRectangle();
	void showOverlay();
	void hideOverlay();
	Graphics::PixelFormat getOverlayFormat() const;
	void clearOverlay();
	void grabOverlay(void *buf, int pitch);
	void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w,
	                       int h);
	virtual int16 getOverlayHeight();
	virtual int16 getOverlayWidth();
	void displayMessageOnOSD(const char *msg) override;
	void displayActivityIconOnOSD(const Graphics::Surface *icon) override;

	bool showMouse(bool visible);
	void warpMouse(int x, int y);
	void setMouseCursor(const void *buf, uint w, uint h, int hotspotX,
	                    int hotspotY, uint32 keycolor, bool dontScale = false,
	                    const Graphics::PixelFormat *format = NULL);
	void setCursorPalette(const byte *colors, uint start, uint num);

	// Transform point from touchscreen coords into gamescreen coords
	void transformPoint(touchPosition &point);
	// Clip point to gamescreen coords
	void clipPoint(touchPosition &point);

	void setCursorDelta(float deltaX, float deltaY);

	void updateFocus();
	void updateMagnify();
	void updateConfig();
	void updateSize();

	void setMagnifyMode(MagnifyMode mode);
	MagnifyMode getMagnifyMode(){ return _gfxState.magMode; }

private:
	void init3DSGraphics();
	void destroy3DSGraphics();
// NEW FUNCTIONS
	bool loadGfx();
	void unloadGfx();
// END NEW
	void initAudio();
	void destroyAudio();
	void initEvents();
	void destroyEvents();
	void runOptionsDialog();

	void flushGameScreen();
	void flushCursor();

protected:
	Audio::MixerImpl *_mixer;

private:
	// Audio
	Thread audioThread;

	// Graphics
	GraphicsModeID _graphicsModeID;
	TransactionState _transactionState;
	TransactionDetails _transactionDetails;

	GfxState _gfxState, _oldGfxState;
	GameState _gameState, _oldGameState;
	CursorState _cursorState;
	MagState _magState;

	byte _palette[3 * 256];
	byte _cursorPalette[3 * 256];

	Graphics::Surface _gameBuffer;
	Sprite _screenBufferTop;
	Sprite _screenBufferBottom;
	Sprite _overlay;
	Sprite _activityIcon;
	Sprite _osdMessage;

	enum {
		kOSDMessageDuration = 800
	};
	uint32 _osdMessageEndTime;

	int _screenShakeXOffset;
	int _screenShakeYOffset;
	bool _overlayVisible;
	int _screenChangeId;

	DVLB_s *_dvlb;
	shaderProgram_s _program;
	int _projectionLocation;
	int _modelviewLocation;
	C3D_Mtx _projectionTop;
	C3D_Mtx _projectionBottom;
	C3D_RenderTarget* _renderTargetTop;
	C3D_RenderTarget* _renderTargetBottom;

	// Focus
	Common::Rect _focusRect;
	bool _focusDirty;
	C3D_Mtx _focusMatrix;
	int _focusPosX, _focusPosY;
	int _focusTargetPosX, _focusTargetPosY;
	float _focusStepPosX, _focusStepPosY;
	float _focusScaleX, _focusScaleY;
	float _focusTargetScaleX, _focusTargetScaleY;
	float _focusStepScaleX, _focusStepScaleY;
	uint32 _focusClearTime;

	// Events
	Thread _eventThread;
	Thread _timerThread;
	Common::Queue<Common::Event> _eventQueue;

	// Cursor
	Graphics::Surface _cursorBuffer;
	Sprite _cursorTexture;

};

} // namespace _3DS

#endif
