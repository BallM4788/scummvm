/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This _program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This _program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this _program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 *
 */

#include "backends/platform/3ds/osystem.h"
#include "backends/platform/3ds/shader_shbin.h"
#include "common/rect.h"
#include "graphics/fontman.h"
#include "gui/gui-manager.h"
#include "options-dialog.h"
#include "config.h"

// Used to transfer the final rendered display to the framebuffer
#define DISPLAY_TRANSFER_FLAGS(in, out)                         \
        (GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) |  \
         GX_TRANSFER_RAW_COPY(0) | GX_TRANSFER_IN_FORMAT(in) |  \
         GX_TRANSFER_OUT_FORMAT(out) |                          \
         GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))
#define TEXTURE_TRANSFER_FLAGS(fmt)                             \
        (GX_TRANSFER_FLIP_VERT(1) | GX_TRANSFER_OUT_TILED(1) |  \
         GX_TRANSFER_RAW_COPY(0) | GX_TRANSFER_IN_FORMAT(fmt) | \
         GX_TRANSFER_OUT_FORMAT(fmt) |                          \
         GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

namespace _3DS {

/* Group the various enums, values, etc. needed for
 * each graphics mode into instaces of GfxMode3DS */
GfxMode3DS _modeRGBA8 = { Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0),
                          GSP_BGR8_OES, GPU_RB_RGBA8, GPU_RGBA8,
                          DISPLAY_TRANSFER_FLAGS(GX_TRANSFER_FMT_RGBA8, GX_TRANSFER_FMT_RGB8),
                          TEXTURE_TRANSFER_FLAGS(GX_TRANSFER_FMT_RGBA8) };
GfxMode3DS _modeRGB565 = { Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0),
                           GSP_RGB565_OES, GPU_RB_RGB565, GPU_RGB565,
                           DISPLAY_TRANSFER_FLAGS(GX_TRANSFER_FMT_RGB565, GX_TRANSFER_FMT_RGB565),
                           TEXTURE_TRANSFER_FLAGS(GX_TRANSFER_FMT_RGB565) };
GfxMode3DS _modeRGB8 = { Graphics::PixelFormat(3, 0, 0, 0, 8, 0, 8, 16, 0),
                         GSP_BGR8_OES, GPU_RB_RGB8, GPU_RGB8,
                         DISPLAY_TRANSFER_FLAGS(GX_TRANSFER_FMT_RGB8, GX_TRANSFER_FMT_RGB8),
                         TEXTURE_TRANSFER_FLAGS(GX_TRANSFER_FMT_RGB8) };
GfxMode3DS _modeRGB555 = { Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0),
                           GSP_RGB5_A1_OES, GPU_RB_RGBA5551, GPU_RGBA5551,
                           DISPLAY_TRANSFER_FLAGS(GX_TRANSFER_FMT_RGB5A1, GX_TRANSFER_FMT_RGB5A1),
                           TEXTURE_TRANSFER_FLAGS(GX_TRANSFER_FMT_RGB5A1) };
GfxMode3DS _modeRGB5A1 = { Graphics::PixelFormat(2, 5, 5, 5, 1, 11, 6, 1, 0),
                           GSP_RGB5_A1_OES, GPU_RB_RGBA5551, GPU_RGBA5551,
                           DISPLAY_TRANSFER_FLAGS(GX_TRANSFER_FMT_RGB5A1, GX_TRANSFER_FMT_RGB5A1),
                           TEXTURE_TRANSFER_FLAGS(GX_TRANSFER_FMT_RGB5A1) };
GfxMode3DS _modeCLUT8 = { Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0),
                          GSP_BGR8_OES, GPU_RB_RGBA8, GPU_RGBA8,
                          DISPLAY_TRANSFER_FLAGS(GX_TRANSFER_FMT_RGBA8, GX_TRANSFER_FMT_RGB8),
                          TEXTURE_TRANSFER_FLAGS(GX_TRANSFER_FMT_RGBA8) };
GfxMode3DS *gfxModes[6] = { &_modeRGBA8, &_modeRGB565, &_modeRGB8,
                            &_modeRGB555, &_modeRGB5A1, &_modeCLUT8 };

// Initiate 3DS graphics when starting up ScummVM
void OSystem_3DS::init3DSGraphics() {
	_gfxState.gfxMode = gfxModes[CLUT8];
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	assert(loadGfx() == true);
}

// Set up Citro3D for the desired graphics mode
bool OSystem_3DS::loadGfx() {
	// Initialize the render targets
	_renderTargetTop =
		C3D_RenderTargetCreate(240, 400, _gfxState.gfxMode->bufferFormat, GPU_RB_DEPTH24_STENCIL8);
	C3D_RenderTargetClear(_renderTargetTop, C3D_CLEAR_ALL, 0x0000000, 0);
	C3D_RenderTargetSetOutput(_renderTargetTop, GFX_TOP, GFX_LEFT, _gfxState.gfxMode->displayTransferFlags);

	_renderTargetBottom =
		C3D_RenderTargetCreate(240, 320, _gfxState.gfxMode->bufferFormat, GPU_RB_DEPTH24_STENCIL8);
	C3D_RenderTargetClear(_renderTargetBottom, C3D_CLEAR_ALL, 0x00000000, 0);
	C3D_RenderTargetSetOutput(_renderTargetBottom, GFX_BOTTOM, GFX_LEFT, _gfxState.gfxMode->displayTransferFlags);

	// Load and bind simple default shader (shader.v.pica)
	_dvlb = DVLB_ParseFile((u32*)shader_shbin, shader_shbin_size);
	shaderProgramInit(&_program);
	shaderProgramSetVsh(&_program, &_dvlb->DVLE[0]);
	C3D_BindProgram(&_program);

	_projectionLocation = shaderInstanceGetUniformLocation(_program.vertexShader, "projection");
	_modelviewLocation = shaderInstanceGetUniformLocation(_program.vertexShader, "modelView");

	C3D_AttrInfo *attrInfo = C3D_GetAttrInfo();
	AttrInfo_Init(attrInfo);
	AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3); // v0=position
	AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 2); // v1=texcoord

	Mtx_OrthoTilt(&_projectionTop, 0.0, 400.0, 240.0, 0.0, 0.0, 1.0, true);
	Mtx_OrthoTilt(&_projectionBottom, 0.0, 320.0, 240.0, 0.0, 0.0, 1.0, true);

	C3D_TexEnv *env = C3D_GetTexEnv(0);
	C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR);
	C3D_TexEnvOpRgb(env, GPU_TEVOP_RGB_SRC_COLOR, GPU_TEVOP_RGB_SRC_COLOR, GPU_TEVOP_RGB_SRC_COLOR);
	C3D_TexEnvFunc(env, C3D_Both, GPU_REPLACE);

	C3D_DepthTest(false, GPU_GEQUAL, GPU_WRITE_ALL);
	C3D_CullFace(GPU_CULL_NONE);
	return true;
}

// Destroy 3DS graphics when exiting ScummVM
void OSystem_3DS::destroy3DSGraphics() {
	_gameBuffer.free();
	_gameTextureTop.free();
	_gameTextureBottom.free();
	_cursorBuffer.free();
	_cursorTexture.free();
	_overlay.free();
	_activityIcon.free();

	unloadGfx();
	C3D_Fini();
}

// Unload the components of the graphics mode no longer in use
void OSystem_3DS::unloadGfx() {
	shaderProgramFree(&_program);
	DVLB_Free(_dvlb);

	C3D_RenderTargetDelete(_renderTargetTop);
	C3D_RenderTargetDelete(_renderTargetBottom);
}

bool OSystem_3DS::hasFeature(OSystem::Feature f) {
	return (f == OSystem::kFeatureCursorPalette ||
	        f == OSystem::kFeatureOverlaySupportsAlpha);
}

void OSystem_3DS::setFeatureState(OSystem::Feature f, bool enable) {
	switch (f) {
	case OSystem::kFeatureCursorPalette:
		_cursorPaletteEnabled = enable;
		flushCursor();
		break;
	default:
		break;
	}
}

bool OSystem_3DS::getFeatureState(OSystem::Feature f) {
	switch (f) {
	case OSystem::kFeatureCursorPalette:
		return _cursorPaletteEnabled;
	default:
		return false;
	}
}

GraphicsModeID OSystem_3DS::chooseMode(Graphics::PixelFormat *format) {
	if (format->bytesPerPixel > 2) {
		if (format->aBits() > 0) {
			return RGBA8;
		} else {
			return RGB8;
		}
	} else if (format->bytesPerPixel > 1) {
		if (format->gBits() > 5) {
			return RGB565;
		} else if (format->aBits() == 0) {
			return RGB555;
		} else {
			return RGB5A1;
		}
	}

	// Last resort
	return CLUT8;
}

bool OSystem_3DS::setGraphicsMode(GraphicsModeID modeID) {
	switch (modeID) {
	case RGBA8:
	case RGB565:
	case RGB8:
	case RGB555:
	case RGB5A1:
	case CLUT8:
		_gfxState.gfxMode = gfxModes[modeID];
		return true;
	default:
		return false;
	}
}

void OSystem_3DS::initSize(uint width, uint height, const Graphics::PixelFormat *format) {
	debug("3ds initsize w:%d h:%d", width, height);
	_gameWidth = width;
	_gameHeight = height;
	_magCenterX = _magWidth / 2;
	_magCenterY = _magHeight / 2;
	_screenShakeXOffset = 0;
	_screenShakeYOffset = 0;

	if (!format) {
		if (_gfxState.gamePF != Graphics::PixelFormat::createFormatCLUT8()) {
			_gfxState.gamePF = Graphics::PixelFormat::createFormatCLUT8();
		}
	} else {
		debug("pixelformat: %d %d %d %d %d", format->bytesPerPixel, format->rBits(), format->gBits(), format->bBits(), format->aBits());
		_gfxState.gamePF = *format;
	}

	/* If the current graphics mode does not fit with the pixel
	 * format being requested, choose one that does and switch to it */
	assert(_gfxState.gamePF.bytesPerPixel > 0);
	if (_gfxState.gamePF != _oldGfxState.gamePF) {
		assert(_transactionState == kTransactionActive);
		_gfxState.gfxModeID = chooseMode(&_gfxState.gamePF);
		_transactionDetails.formatChanged = true;
	}

	int oldScreen = config.screen;
	loadConfig();
	if (config.screen != oldScreen) {
		_screenChangeId++;
	}

	_gameTextureTop.create(width, height, _gfxState.gfxMode);
	_overlay.create(400, 320, _gfxState.gfxMode);
	_gameBuffer.create(width, height, _gfxState.gamePF);

	_focusDirty = true;
	_focusRect = Common::Rect(_gameWidth, _gameHeight);

	updateSize();
}

void OSystem_3DS::updateSize() {
	if (config.stretchToFit) {
		_gameTopX = _gameTopY = _gameBottomX = _gameBottomY = 0;
		_gameTextureTop.setScale(400.f / _gameWidth, 240.f / _gameHeight);
		_gameTextureBottom.setScale(320.f / _gameWidth, 240.f / _gameHeight);
	} else {
		float ratio = static_cast<float>(_gameWidth) / _gameHeight;

		if (ratio > 400.f / 240.f) {
			float r = 400.f / _gameWidth;
			_gameTextureTop.setScale(r, r);
			_gameTopX = 0;
			_gameTopY = (240.f / r - _gameHeight) / 2.f;
		} else {
			float r = 240.f / _gameHeight;
			_gameTextureTop.setScale(r, r);
			_gameTopY = 0;
			_gameTopX = (400.f / r - _gameWidth) / 2.f;
		}
		if (ratio > 320.f / 240.f) {
			float r = 320.f / _gameWidth;
			_gameTextureBottom.setScale(r, r);
			_gameBottomX = 0;
			_gameBottomY = (240.f / r - _gameHeight) / 2.f;
		} else {
			float r = 240.f / _gameHeight;
			_gameTextureBottom.setScale(r, r);
			_gameBottomY = 0;
			_gameBottomX = (320.f / r - _gameWidth) / 2.f;
		}
	}
	_gameTextureTop.setPosition(_gameTopX, _gameTopY);
	_gameTextureBottom.setPosition(_gameBottomX, _gameBottomY);
	_gameTextureTop.setOffset(0, 0);
	_gameTextureBottom.setOffset(0, 0);
	if (_overlayVisible) {
		_cursorTexture.setScale(1.f, 1.f);
	} else if (config.screen == kScreenTop) {
		_cursorTexture.setScale(_gameTextureTop.getScaleX(), _gameTextureTop.getScaleY());
	} else {
		_cursorTexture.setScale(_gameTextureBottom.getScaleX(), _gameTextureBottom.getScaleY());
	}
}

Common::List<Graphics::PixelFormat> OSystem_3DS::getSupportedFormats() const {
	Common::List<Graphics::PixelFormat> list;
	list.push_back(_modeRGBA8.svmPF); // GPU_RGBA8
	list.push_back(_modeRGB565.svmPF); // GPU_RGB565
//	list.push_back(_modeRGB8.svmPF); // GPU_RGB8
	list.push_back(_modeRGB555.svmPF); // RGB555 (needed for FMTOWNS?)
	list.push_back(_modeRGB5A1.svmPF); // GPU_RGBA5551
	list.push_back(Graphics::PixelFormat::createFormatCLUT8());
	return list;
}

void OSystem_3DS::beginGFXTransaction() {
	assert(_transactionState == kTransactionNone);
	_transactionState = kTransactionActive;
	_transactionDetails.formatChanged = false;
	_oldGfxState = _gfxState;
}

OSystem::TransactionError OSystem_3DS::endGFXTransaction() {
	int errors = OSystem::kTransactionSuccess;

	assert(_transactionState != kTransactionNone);
	if (_transactionState == kTransactionRollback) {
		if ((_gfxState.gfxMode != _oldGfxState.gfxMode) |
		    (_gfxState.gfxModeID != _oldGfxState.gfxModeID) |
		    (_gfxState.gfxMode != gfxModes[_gfxState.gfxModeID])) {
			errors |= OSystem::kTransactionFormatNotSupported;
			_gfxState = _oldGfxState;
		}

		_oldGfxState.setup = false;
	}
	if (_transactionDetails.formatChanged) {
		if (!setGraphicsMode(_gfxState.gfxModeID)) {
			if (_oldGfxState.setup) {
				_transactionState = kTransactionRollback;
				errors |= endGFXTransaction();
			}
		} else if (_gfxState.gfxMode != gfxModes[_gfxState.gfxModeID]){
			if (_oldGfxState.setup) {
				_transactionState = kTransactionRollback;
				errors |= endGFXTransaction();
			}
		} else {
			unloadGfx();
			gfxSetScreenFormat(GFX_TOP, _gfxState.gfxMode->screenFormat);
			gfxSetScreenFormat(GFX_BOTTOM, _gfxState.gfxMode->screenFormat);
			if (!loadGfx()) {
				if (_oldGfxState.setup) {
					_transactionState = kTransactionRollback;
					errors |= endGFXTransaction();
				}
			} else {
				initSize(_gameWidth, _gameHeight, &_gfxState.gamePF);
				clearOverlay();
				_gfxState.setup = true;
				_screenChangeId++;
			}
		}
	}

	_transactionState = kTransactionNone;
	return (OSystem::TransactionError)errors;
}

float OSystem_3DS::getScaleRatio() const {
	if (_overlayVisible) {
		return 1.0;
	} else if (config.screen == kScreenTop) {
		return _gameTextureTop.getScaleX();
	} else {
		return _gameTextureBottom.getScaleX();
	}
}

void OSystem_3DS::setPalette(const byte *colors, uint start, uint num) {
	assert(start + num <= 256);
	memcpy(_palette + 3 * start, colors, 3 * num);

	// Manually update all color that were changed
	if (_gameBuffer.format.bytesPerPixel == 1) {
		flushGameScreen();
	}
}
void OSystem_3DS::grabPalette(byte *colors, uint start, uint num) const {
	assert(start + num <= 256);
	memcpy(colors, _palette + 3 * start, 3 * num);
}

void OSystem_3DS::copyRectToScreen(const void *buf, int pitch,
                                   int x, int y, int w, int h) {
	Common::Rect rect(x, y, x+w, y+h);
	_gameBuffer.copyRectToSurface(buf, pitch, x, y, w, h);
	Graphics::Surface subSurface = _gameBuffer.getSubArea(rect);

	Graphics::Surface *convertedSubSurface = subSurface.convertTo(_gfxState.gfxMode->svmPF, _palette);
	_gameTextureTop.copyRectToSurface(*convertedSubSurface, x, y, Common::Rect(w, h));

	convertedSubSurface->free();
	delete convertedSubSurface;
	_gameTextureTop.markDirty();
}

void OSystem_3DS::flushGameScreen() {
	Graphics::Surface *converted = _gameBuffer.convertTo(_gfxState.gfxMode->svmPF, _palette);
	_gameTextureTop.copyRectToSurface(*converted, 0, 0, Common::Rect(converted->w, converted->h));
	_gameTextureTop.markDirty();
	converted->free();
	delete converted;
}

Graphics::Surface *OSystem_3DS::lockScreen() {
	return &_gameBuffer;
}
void OSystem_3DS::unlockScreen() {
	flushGameScreen();
}

void OSystem_3DS::updateScreen() {
	if (sleeping || exiting)
		return;

//	updateFocus();
	updateMagnify();

	if (_gfxState.gfxMode->svmPF.aBits() == 0) {
		flushCursor();
	}

	if (_osdMessage.getPixels() && _osdMessageEndTime <= getMillis(true)) {
		_osdMessage.free();
	}

	C3D_FrameBegin(0);
		_gameTextureTop.transfer();
		if (_overlayVisible) {
			_overlay.transfer();
		}
		if (_cursorVisible && config.showCursor) {
			_cursorTexture.transfer();
		}
		_osdMessage.transfer();
		_activityIcon.transfer();
	C3D_FrameEnd(0);

	C3D_FrameBegin(0);
		// Render top screen
		C3D_RenderTargetClear(_renderTargetTop, C3D_CLEAR_ALL, 0x00000000, 0);
		C3D_FrameDrawOn(_renderTargetTop);
		if (config.screen == kScreenTop || config.screen == kScreenBoth) {
			C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, _projectionLocation, &_projectionTop);
			C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, _modelviewLocation, _gameTextureTop.getMatrix());
			_gameTextureTop.render();
			if (_overlayVisible && config.screen == kScreenTop) {
				C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, _modelviewLocation, _overlay.getMatrix());
				_overlay.render();
			}
			if (_activityIcon.getPixels() && config.screen == kScreenTop) {
				_activityIcon.setPosition(400 - _activityIcon.actualWidth, 0);
				C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, _modelviewLocation, _activityIcon.getMatrix());
				_activityIcon.render();
			}
			if (_osdMessage.getPixels() && config.screen == kScreenTop) {
				_osdMessage.setPosition((400 - _osdMessage.actualWidth) / 2, (240 - _osdMessage.actualHeight) / 2);
				C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, _modelviewLocation, _osdMessage.getMatrix());
				_osdMessage.render();
			}
			if (_cursorVisible && config.showCursor && config.screen == kScreenTop) {
				C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, _modelviewLocation, _cursorTexture.getMatrix());
				_cursorTexture.render();
			}
		}

		// Render bottom screen
		C3D_RenderTargetClear(_renderTargetBottom, C3D_CLEAR_ALL, 0x00000000, 0);
		C3D_FrameDrawOn(_renderTargetBottom);
		if (config.screen == kScreenBottom || config.screen == kScreenBoth) {
			C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, _projectionLocation, &_projectionBottom);
			C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, _modelviewLocation, _gameTextureBottom.getMatrix());
			_gameTextureTop.render();
			if (_overlayVisible) {
				C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, _modelviewLocation, _overlay.getMatrix());
				_overlay.render();
			}
			if (_activityIcon.getPixels()) {
				_activityIcon.setPosition(320 - _activityIcon.actualWidth, 0);
				C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, _modelviewLocation, _activityIcon.getMatrix());
				_activityIcon.render();
			}
			if (_osdMessage.getPixels()) {
				_osdMessage.setPosition((320 - _osdMessage.actualWidth) / 2, (240 - _osdMessage.actualHeight) / 2);
				C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, _modelviewLocation, _osdMessage.getMatrix());
				_osdMessage.render();
			}
			if (_cursorVisible && config.showCursor) {
				C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, _modelviewLocation, _cursorTexture.getMatrix());
				_cursorTexture.render();
			}
		}
	C3D_FrameEnd(0);
}

void OSystem_3DS::setShakePos(int shakeXOffset, int shakeYOffset) {
	// TODO: implement this in overlay, top screen, and mouse too
	_screenShakeXOffset = shakeXOffset;
	_screenShakeYOffset = shakeYOffset;
	int topX = _gameTopX + (_gameTextureTop.getScaleX() * shakeXOffset);
	int topY = _gameTopY + (_gameTextureTop.getScaleY() * shakeYOffset);
	_gameTextureTop.setPosition(topX, topY);
	int bottomX = _gameBottomX + (_gameTextureBottom.getScaleX() * shakeXOffset);
	int bottomY = _gameBottomY + (_gameTextureBottom.getScaleY() * shakeYOffset);
	_gameTextureBottom.setPosition(bottomX, bottomY);
}

void OSystem_3DS::setFocusRectangle(const Common::Rect &rect) {
	debug("setfocus: %d %d %d %d", rect.left, rect.top, rect.width(), rect.height());
	_focusRect = rect;
	_focusDirty = true;
	_focusClearTime = 0;
}

void OSystem_3DS::clearFocusRectangle() {
	_focusClearTime = getMillis();
}

void OSystem_3DS::updateFocus() {

	if (_focusClearTime && getMillis() - _focusClearTime > 5000) {
		_focusClearTime = 0;
		_focusDirty = true;
		_focusRect = Common::Rect(_gameWidth, _gameHeight);
	}

	if (_focusDirty) {
		float duration = 1.f / 20.f; // Focus animation in frame duration
		float w = 400.f;
		float h = 240.f;
		float ratio = _focusRect.width() / _focusRect.height();
		if (ratio > w/h) {
			_focusTargetScaleX = w / _focusRect.width();
			float newHeight = (float)_focusRect.width() / w/h;
			_focusTargetScaleY = h / newHeight;
			_focusTargetPosX = _focusTargetScaleX * _focusRect.left;
			_focusTargetPosY = _focusTargetScaleY * ((float)_focusRect.top - (newHeight - _focusRect.height())/2.f);
		} else {
			_focusTargetScaleY = h / _focusRect.height();
			float newWidth = (float)_focusRect.height() * w/h;
			_focusTargetScaleX = w / newWidth;
			_focusTargetPosY = _focusTargetScaleY * _focusRect.top;
			_focusTargetPosX = _focusTargetScaleX * ((float)_focusRect.left - (newWidth - _focusRect.width())/2.f);
		}
		if (_focusTargetPosX < 0 && _focusTargetScaleY != 240.f / _gameHeight) {
			_focusTargetPosX = 0;
		}
		if (_focusTargetPosY < 0 && _focusTargetScaleX != 400.f / _gameWidth) {
			_focusTargetPosY = 0;
		}
		_focusStepPosX = duration * (_focusTargetPosX - _focusPosX);
		_focusStepPosY = duration * (_focusTargetPosY - _focusPosY);
		_focusStepScaleX = duration * (_focusTargetScaleX - _focusScaleX);
		_focusStepScaleY = duration * (_focusTargetScaleY - _focusScaleY);
	}

	if (_focusDirty || _focusPosX != _focusTargetPosX || _focusPosY != _focusTargetPosY ||
	    _focusScaleX != _focusTargetScaleX || _focusScaleY != _focusTargetScaleY) {
		_focusDirty = false;

		if ((_focusStepPosX > 0 && _focusPosX > _focusTargetPosX) || (_focusStepPosX < 0 && _focusPosX < _focusTargetPosX)) {
			_focusPosX = _focusTargetPosX;
		} else if (_focusPosX != _focusTargetPosX) {
			_focusPosX += _focusStepPosX;
		}

		if ((_focusStepPosY > 0 && _focusPosY > _focusTargetPosY) || (_focusStepPosY < 0 && _focusPosY < _focusTargetPosY)) {
			_focusPosY = _focusTargetPosY;
		} else if (_focusPosY != _focusTargetPosY) {
			_focusPosY += _focusStepPosY;
		}

		if ((_focusStepScaleX > 0 && _focusScaleX > _focusTargetScaleX) || (_focusStepScaleX < 0 && _focusScaleX < _focusTargetScaleX)) {
			_focusScaleX = _focusTargetScaleX;
		} else if (_focusScaleX != _focusTargetScaleX) {
			_focusScaleX += _focusStepScaleX;
		}

		if ((_focusStepScaleY > 0 && _focusScaleY > _focusTargetScaleY) || (_focusStepScaleY < 0 && _focusScaleY < _focusTargetScaleY)) {
			_focusScaleY = _focusTargetScaleY;
		} else if (_focusScaleY != _focusTargetScaleY) {
			_focusScaleY += _focusStepScaleY;
		}

		Mtx_Identity(&_focusMatrix);
		Mtx_Translate(&_focusMatrix, -_focusPosX, -_focusPosY, 0, true);
		Mtx_Scale(&_focusMatrix, _focusScaleX, _focusScaleY, 1.f);
	}
}

void OSystem_3DS::updateMagnify() {
	if (_gfxState.magMode == MODE_MAGON && config.screen != kScreenBoth) {
		// Only allow to magnify when both screens are enabled
		_gfxState.magMode = MODE_MAGOFF;
	}

	if (_gfxState.magMode == MODE_MAGON) {
		if (!g_gui.isActive()) {
			_magX = (_cursorXScreen < _magCenterX) ?
			         0 : ((_cursorXScreen < (_gameWidth - _magCenterX)) ?
			         _cursorXScreen - _magCenterX : _gameWidth - _magWidth);
			_magY = (_cursorYScreen < _magCenterY) ?
			         0 : ((_cursorYScreen < _gameHeight - _magCenterY) ?
			         _cursorYScreen - _magCenterY : _gameHeight - _magHeight);
		}
		_gameTextureTop.setScale(1.f,1.f);
		_gameTextureTop.setPosition(0,0);
		_gameTextureTop.setOffset(_magX, _magY);
	}
}

void OSystem_3DS::showOverlay() {
	_overlayVisible = true;
	updateSize();
}

void OSystem_3DS::hideOverlay() {
	_overlayVisible = false;
	updateSize();
}

Graphics::PixelFormat OSystem_3DS::getOverlayFormat() const {
	return _overlay.format;
}

void OSystem_3DS::clearOverlay() {
	_overlay.clear();
}

void OSystem_3DS::grabOverlay(void *buf, int pitch) {
	byte *dst = (byte *)buf;

	for (int y = 0; y < getOverlayHeight(); ++y) {
		memcpy(dst, _overlay.getBasePtr(0, y), getOverlayWidth() * _gfxState.gfxMode->svmPF.bytesPerPixel);
		dst += pitch;
	}
}

void OSystem_3DS::copyRectToOverlay(const void *buf, int pitch,
                                    int x, int y, int w, int h) {
	_overlay.copyRectToSurface(buf, pitch, x, y, w, h);
	_overlay.markDirty();
}

void OSystem_3DS::displayMessageOnOSD(const char *msg) {
	// The font we are going to use:
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kLocalizedFont);
	if (!font) {
		warning("No available font to render OSD messages");
		return;
	}

	// Split the message into separate lines.
	Common::Array<Common::String> lines;
	const char *ptr;
	for (ptr = msg; *ptr; ++ptr) {
		if (*ptr == '\n') {
			lines.push_back(Common::String(msg, ptr - msg));
			msg = ptr + 1;
		}
	}
	lines.push_back(Common::String(msg, ptr - msg));

	// Determine a rect which would contain the message string (clipped to the
	// screen dimensions).
	const int vOffset = 6;
	const int lineSpacing = 1;
	const int lineHeight = font->getFontHeight() + 2 * lineSpacing;
	int width = 0;
	int height = lineHeight * lines.size() + 2 * vOffset;
	uint i;
	for (i = 0; i < lines.size(); i++) {
		width = MAX(width, font->getStringWidth(lines[i]) + 14);
	}

	// Clip the rect
	if (width > getOverlayWidth())
		width = getOverlayWidth();
	if (height > getOverlayHeight())
		height = getOverlayHeight();

	_osdMessage.create(width, height, _gfxState.gfxMode);
	_osdMessage.fillRect(Common::Rect(width, height), _gfxState.gfxMode->svmPF.ARGBToColor(200, 0, 0, 0));

	// Render the message, centered, and in white
	for (i = 0; i < lines.size(); i++) {
		font->drawString(&_osdMessage, lines[i],
		                 0, 0 + i * lineHeight + vOffset + lineSpacing, width,
		                 _gfxState.gfxMode->svmPF.RGBToColor(255, 255, 255),
		                 Graphics::kTextAlignCenter);
	}

	_osdMessageEndTime = getMillis(true) + kOSDMessageDuration;
}

void OSystem_3DS::displayActivityIconOnOSD(const Graphics::Surface *icon) {
	if (!icon) {
		_activityIcon.free();
	} else {
		if (!_activityIcon.getPixels() || icon->w != _activityIcon.w || icon->h != _activityIcon.h) {
			_activityIcon.create(icon->w, icon->h, _gfxState.gfxMode);
		}

		Graphics::Surface *converted = icon->convertTo(_gfxState.gfxMode->svmPF);
		_activityIcon.copyRectToSurface(*converted, 0, 0, Common::Rect(converted->w, converted->h));
		_activityIcon.markDirty();
		converted->free();
		delete converted;
	}
}

int16 OSystem_3DS::getOverlayHeight() {
	return 240;
}

int16 OSystem_3DS::getOverlayWidth() {
	return config.screen == kScreenTop ? 400 : 320;
}

bool OSystem_3DS::showMouse(bool visible) {
	_cursorVisible = visible;
	flushCursor();
	return !visible;
}

void OSystem_3DS::warpMouse(int x, int y) {
	if (!g_gui.isActive()) {
		_cursorXScreen = x;
		_cursorYScreen = y;
	} else {
		_cursorXOverlay = x;
		_cursorYOverlay = y;
	}

	// TODO: adjust for _cursorScalable ?
	x -= _cursorHotspotX;
	y -= _cursorHotspotY;

	int offsetx = 0;
	int offsety = 0;
	if (!_overlayVisible) {
		offsetx = config.screen == kScreenTop ? _gameTextureTop.getPosX() : _gameTextureBottom.getPosX();
		offsety = config.screen == kScreenTop ? _gameTextureTop.getPosY() : _gameTextureBottom.getPosY();
	}

	_cursorTexture.setPosition(x + offsetx, y + offsety);

	if ( (_cursorTexture.getPosLastX() != _cursorTexture.getPosX()) |
	     (_cursorTexture.getPosLastY() != _cursorTexture.getPosY()) ) {
		int cursorLeft = x;
		int cursorTop = y;
		int cursorRight = cursorLeft + _cursorTexture.actualWidth;
		int cursorBottom = cursorTop + _cursorTexture.actualHeight;
		_cursorSpace = Common::Rect(cursorLeft, cursorTop, cursorRight, cursorBottom);
		if (_gfxState.gfxMode->svmPF.aBits() == 0) {
			flushCursor();
		}
	}
}

void OSystem_3DS::setCursorDelta(float deltaX, float deltaY) {
	_cursorDeltaX = deltaX;
	_cursorDeltaY = deltaY;
}

void OSystem_3DS::setMouseCursor(const void *buf, uint w, uint h,
                                 int hotspotX, int hotspotY,
                                 uint32 keycolor, bool dontScale,
                                 const Graphics::PixelFormat *format) {
	_cursorScalable = !dontScale;
	_cursorHotspotX = hotspotX;
	_cursorHotspotY = hotspotY;
	_cursorKeyColor = keycolor;
	_pfCursor = !format ? Graphics::PixelFormat::createFormatCLUT8() : *format;

	if (w != _cursorBuffer.w || h != _cursorBuffer.h || _cursorBuffer.format != _pfCursor) {
		_cursorBuffer.create(w, h, _pfCursor);
		_cursorTexture.create(w, h, _gfxState.gfxMode);
		_cursorSpace = Common::Rect(_cursorTexture.getPosX(), _cursorTexture.getPosY(),
		                            _cursorTexture.getPosX() + w, _cursorTexture.getPosY() + h);
	}

	if ( w != 0 && h != 0 ) {
		_cursorBuffer.copyRectToSurface(buf, w * _pfCursor.bytesPerPixel, 0, 0, w, h);
	}

	(!g_gui.isActive()) ? warpMouse(_cursorXScreen, _cursorYScreen) : warpMouse(_cursorXOverlay, _cursorYOverlay);
	flushCursor();
}

void OSystem_3DS::setCursorPalette(const byte *colors, uint start, uint num) {
	assert(start + num <= 256);
	memcpy(_cursorPalette + 3 * start, colors, 3 * num);
	_cursorPaletteEnabled = true;
	flushCursor();
}

namespace {
template<typename SrcColor, typename DstColor>
void applyKeyColor(Graphics::Surface &src, Sprite &dst, Sprite &screen, const SrcColor keyColor, Common::Rect area) {
	assert((dst.format.bytesPerPixel == 4) | (dst.format.bytesPerPixel == 2));
	assert((dst.w >= src.w) && (dst.h >= src.h));

	Graphics::Surface cursorSpaceOnScreen;
	DstColor *screenPtr = NULL;

	if (dst.format.aBits() == 0) {
		cursorSpaceOnScreen = screen.getSubArea(area);
	}
	for (uint y = 0; y < src.h; ++y) {
		const SrcColor *srcPtr = (const SrcColor *)src.getBasePtr(0, y);
		DstColor *dstPtr = (DstColor *)dst.getBasePtr(0, y);
		if (dst.format.aBits() == 0) {
			screenPtr = (DstColor *)cursorSpaceOnScreen.getBasePtr(0, y);
		}

		for (uint x = 0; x < src.w; ++x, srcPtr++, dstPtr++) {
			if (*srcPtr == keyColor) {
				*dstPtr = (dst.format.aBits() != 0) ? 0 : *screenPtr;
			}

			if (dst.format.aBits() == 0) {
				screenPtr++;
			}
		}
	}
}
} // End of anonymous namespace

void OSystem_3DS::flushCursor() {
	if (_cursorBuffer.getPixels()) {
		Graphics::Surface *converted = _cursorBuffer.convertTo(_gfxState.gfxMode->svmPF, _cursorPaletteEnabled ? _cursorPalette : _palette);
		_cursorTexture.copyRectToSurface(*converted, 0, 0, Common::Rect(converted->w, converted->h));
		converted->free();
		delete converted;

		if (_gfxState.gfxMode->svmPF.bytesPerPixel == 4) {
			if (_pfCursor.bytesPerPixel == 1) {
				applyKeyColor<byte, uint32>(_cursorBuffer, _cursorTexture, (!g_gui.isActive()) ? _gameTextureTop : _overlay,
				                            _cursorKeyColor, _cursorSpace);
			} else if (_pfCursor.bytesPerPixel == 2) {
				applyKeyColor<uint16, uint32>(_cursorBuffer, _cursorTexture, (!g_gui.isActive()) ? _gameTextureTop : _overlay,
				                              _cursorKeyColor, _cursorSpace);
			} else if (_pfCursor.bytesPerPixel == 4) {
				applyKeyColor<uint32, uint32>(_cursorBuffer, _cursorTexture, (!g_gui.isActive()) ? _gameTextureTop : _overlay,
				                              _cursorKeyColor, _cursorSpace);
			}
		} else if (_gfxState.gfxMode->svmPF.bytesPerPixel == 2) {
			if (_pfCursor.bytesPerPixel == 1) {
				applyKeyColor<byte, uint16>(_cursorBuffer, _cursorTexture, (!g_gui.isActive()) ? _gameTextureTop : _overlay,
				                            _cursorKeyColor, _cursorSpace);
			} else if (_pfCursor.bytesPerPixel == 2) {
				applyKeyColor<uint16, uint16>(_cursorBuffer, _cursorTexture, (!g_gui.isActive()) ? _gameTextureTop : _overlay,
				                              _cursorKeyColor, _cursorSpace);
			} else if (_pfCursor.bytesPerPixel == 4) {
				applyKeyColor<uint32, uint16>(_cursorBuffer, _cursorTexture, (!g_gui.isActive()) ? _gameTextureTop : _overlay,
				                              _cursorKeyColor, _cursorSpace);
			}
		}
		_cursorTexture.markDirty();
	}
}

} // namespace _3DS
