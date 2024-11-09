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

#ifndef GRAPHICS_3DS_NATIVE3D_H
#define GRAPHICS_3DS_NATIVE3D_H

//struct Tex3DS_Texture_s
//{
//	u16 numSubTextures;               // Number of subtextures
//	u16 width;                        // Texture width
//	u16 height;                       // Texture height
//	u8  format;                       // Texture format
//	u8  mipmapLevels;                 // Number of mipmaps
//	Tex3DS_SubTexture *subTextures;   // Subtextures
//};

#include <3ds.h>
#include <citro3d.h>
#include "common/system.h"
#include "graphics/3ds/n3d-context.h"
//#include "graphics/3ds/n3d-shaderobj.h"

namespace N3DS_3D {

class Native3D {
public:
	Native3D();
	~Native3D();

	N3DContext *getContext();
	void setContext(N3DContext *context);
	void saveContext(N3DContext *contextDst = nullptr);
	void restoreContext(N3DContext *contextSrc = nullptr);
	void defaultContext();

	void changeShader(ShaderObj *shaderObj);

	C3D_Tex *getGameScreen();


	void arbDataToArbBlockTexOffset(u32 *srcBuf, u32 *dstBuf, int copyWidth, int copyHeight,
	                                int xSource, int ySource, int wSource,   int hSource,
	                                int xDest,   int yDest,   int wDest,     int hDest,
	                                GPU_TEXCOLOR format, int scale, bool isBlockSrc);

	void dataToBlockTex(u32 *srcBuf, u32 *dstBuf, int xSource, int ySource,
	                    int wSource, int hSource, int wDest, int hDest,
	                    GPU_TEXCOLOR format, int scale, bool isBlockSrc);


	void viewport(u32 x, u32 y, u32 w, u32 h);

	void cullFace(N3D_CULLFACE mode);
	void frontFace(N3D_FRONTFACE mode);
	void cullFaceEnabled(bool state);

	void polygonOffset(float pOffUnits);
	void wScale(float wScale);
	void depthRange(float near, float far);
	void polygonOffsetEnabled(bool state);

	void scissorMode(GPU_SCISSORMODE mode);
	void scissorDims(u32 x, u32 y, u32 w, u32 h);

	void alphaFunc(GPU_TESTFUNC func, int ref);
	void alphaTestEnabled(bool state);

	void stencilFunc(GPU_TESTFUNC func, int ref, int mask);
	void stencilMask(int writeMask);
	void stencilOp(GPU_STENCILOP fail, GPU_STENCILOP zfail, GPU_STENCILOP zpass);
	// CLEAR STENCIL????????????
	void stencilTestEnabled(bool state);

	void depthFunc(GPU_TESTFUNC func);
	void depthMask(bool depthState);
	void colorMask(bool redState, bool greenState, bool blueState, bool alphaState);
	// CLEAR DEPTH?????????????
	void depthTestEnabled(bool state);

	void earlyDepthFunc(GPU_EARLYDEPTHFUNC func);
	void clearEarlyDepth(u32 clearValue);
	void earlyDepthTestEnabled(bool state);

	void blendEquation(GPU_BLENDEQUATION equation);
	void blendEquationSeparate(GPU_BLENDEQUATION colorEq, GPU_BLENDEQUATION alphaEq);
	void blendFunc(GPU_BLENDFACTOR srcFactor, GPU_BLENDFACTOR dstFactor);
	void blendFuncSeparate(GPU_BLENDFACTOR srcColor, GPU_BLENDFACTOR dstColor,
	                       GPU_BLENDFACTOR srcAlpha, GPU_BLENDFACTOR dstAlpha);
	void blendColor(u8 red, u8 green, u8 blue, u8 alpha);
	void blendEnabled(bool state);
	// CLEAR COLOR???????????????

	void logicOp(GPU_LOGICOP op);
	void colorLogicOpEnabled(bool state);

private:
//	u32 getPixelSizeInBytes(GPU_TEXCOLOR format, u32 *isHalfByte = nullptr);
//	u32 mortonInterleave(u32 x, u32 y);
//	u32 mortonOffset(u32 x, u32 y);
	void updateCullMode();
	void updateDepthMap();
	void updateBlend();
	void updateEntireContext();

public:
	N3DContext  _defaultContext,
	            _savedContext;
	N3DContext *_activeContext;
};

} // end of namespace N3DS_3D

#endif // GRAPHICS_N3DS_NATIVE3D_H