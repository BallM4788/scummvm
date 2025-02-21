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

// from "graphics/3ds/ops-gl2citro.cpp"
void opCullFace(N3D_CULLFACE mode);
void opFrontFace(N3D_FRONTFACE mode);
void opCullFaceEnabled(bool state);
void opPolygonOffset(float pOffUnits);
void opDepthRange(float near, float far);
void opPolygonOffsetEnabled(bool state);
void opScissorMode(GPU_SCISSORMODE mode);
void opScissorDims(u32 x, u32 y, u32 w, u32 h);
void opAlphaFunc(GPU_TESTFUNC func, int ref);
void opAlphaTestEnabled(bool state);
void opStencilFunc(GPU_TESTFUNC func, int ref, int mask);
void opStencilMask(int writeMask);
void opStencilOp(GPU_STENCILOP fail, GPU_STENCILOP zfail, GPU_STENCILOP zpass);
// CLEAR STENCIL????????????
void opStencilTestEnabled(bool state);
void opDepthFunc(GPU_TESTFUNC func);
void opDepthMask(bool depthState);
void opColorMask(bool redState, bool greenState, bool blueState, bool alphaState);
// CLEAR DEPTH?????????????
void opDepthTestEnabled(bool state);
void opEarlyDepthFunc(GPU_EARLYDEPTHFUNC func);
void opClearEarlyDepth(u32 clearValue);
void opEarlyDepthTestEnabled(bool state);
void opBlendEquation(GPU_BLENDEQUATION equation);
void opBlendEquationSeparate(GPU_BLENDEQUATION colorEq, GPU_BLENDEQUATION alphaEq);
void opBlendFunc(GPU_BLENDFACTOR srcFactor, GPU_BLENDFACTOR dstFactor);
void opBlendFuncSeparate(GPU_BLENDFACTOR srcColor, GPU_BLENDFACTOR dstColor, GPU_BLENDFACTOR srcAlpha, GPU_BLENDFACTOR dstAlpha);
void opBlendColor(u8 red, u8 green, u8 blue, u8 alpha);
void opBlendEnabled(bool state);
// CLEAR COLOR???????????????
void opLogicOp(GPU_LOGICOP op);
void opColorLogicOpEnabled(bool state);


// CUSTOM FUNCS
C3D_Tex         *opGetGameScreen();
void            *opCreateBuffer(size_t size, const void *data = nullptr, size_t alignment = 0x80);
void             opFreeBuffer(void *linearBuffer);
void             opArbDataToArbBlockTexOffset(u32 *srcBuf, u32 *dstBuf, int copyWidth, int copyHeight,
                                              int xSource, int ySource, int wSource,   int hSource,
                                              int xDest,   int yDest,   int wDest,     int hDest,
                                              GPU_TEXCOLOR format, bool isBlockSrc);
void             opDataToBlockTex(u32 *srcBuf, u32 *dstBuf, int xSource, int ySource,
                                  int wSource, int hSource, int wDest, int hDest,
                                  GPU_TEXCOLOR format, bool isBlockSrc);
