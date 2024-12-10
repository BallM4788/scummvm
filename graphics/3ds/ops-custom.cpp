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

#include "graphics/3ds/z3d.h"

namespace N3DS_3D {

C3D_Tex *N3DContext::opGetGameScreen() {
	N3DS::OSystem_3DS *gsys3DS = reinterpret_cast<N3DS::OSystem_3DS *>(g_system);
	return gsys3DS->getGameSurface();
}

static u32 getPixelSizeInBytes(GPU_TEXCOLOR format, u32 *isHalfByte) {
	switch(format) {
		case GPU_RGBA8:
			return 4;
		case GPU_RGB8:
			return 3;
		case GPU_LA8:
		case GPU_HILO8:
		case GPU_RGB565:
		case GPU_RGBA5551:
		case GPU_RGBA4:
			return 2;
		case GPU_A8:
		case GPU_L8:
		case GPU_LA4:
			return 1;
		case GPU_L4:
		case GPU_A4:
			if (isHalfByte != nullptr) *isHalfByte = 1;
			return 1;
		default:
			return 0;
	}
}

static u32 mortonInterleave(u32 x, u32 y) {
	static u32 xlut[] = {0x00, 0x01, 0x04, 0x05, 0x10, 0x11, 0x14, 0x15};
	static u32 ylut[] = {0x00, 0x02, 0x08, 0x0a, 0x20, 0x22, 0x28, 0x2a};

	return xlut[x % 8] + ylut[y % 8];
}

static u32 mortonOffset(u32 x, u32 y) {
	u32 i = mortonInterleave(x, y);
	u32 xCoarse = x & ~7;
	u32 offset = xCoarse << 3;

	return (i + offset);
}

void N3DContext::opArbDataToArbBlockTexOffset(u32 *srcBuf, u32 *dstBuf, int copyWidth, int copyHeight,
                                              int xSource, int ySource, int wSource,   int hSource,
                                              int xDest,   int yDest,   int wDest,     int hDest,
                                              GPU_TEXCOLOR format, int scale, bool isBlockSrc) {
	u32 pixSize = 4, halfByte = 0;
	int yFlip   = !isBlockSrc;

	pixSize = getPixelSizeInBytes(format, &halfByte);

	u32 wSrc = wSource  << scale;
	u32 hSrc = hSource << scale;
	u32 wDst = wDest  << scale;
	u32 hDst = hDest << scale;
	u32 copyW = copyWidth << scale;
	u32 copyH = copyHeight << scale;
	u32 ySrc = hSrc - (ySource << scale) - copyH;
	u32 xSrc = xSource << scale;
	u32 yDst = hDst - (yDest << scale) - copyH;
	u32 xDst = xDest << scale;
	u32 x, y;
	u8 *srcConverted;

	if (!isBlockSrc) {
		int idx;
		int size = wSource * hSource * pixSize;
		if (halfByte) size /= 2;

		srcConverted = (u8 *)linearAlloc((size_t)size);

		// some color formats have reversed component positions under the 3DS's native texture format
		u16 *srcBuf16;
		switch (format) {
			case GPU_RGBA8:
				for (idx = 0; idx < wSource * hSource; idx++) {
					((u32 *)srcConverted)[idx] = (((srcBuf[idx] << 24) & 0xff000000) |
					                              ((srcBuf[idx] << 8)  & 0x00ff0000) |
					                              ((srcBuf[idx] >> 8)  & 0x0000ff00) |
					                              ((srcBuf[idx] >> 24) & 0x000000ff));
				}
				break;
			case GPU_RGB8:
				for (idx = 0; idx < wSource * hSource; idx++) {
					int pxl = idx * 3;
					srcConverted[pxl + 2] = ((u8 *)srcBuf)[pxl + 0];
					srcConverted[pxl + 1] = ((u8 *)srcBuf)[pxl + 1];
					srcConverted[pxl + 0] = ((u8 *)srcBuf)[pxl + 2];
				}
				break;
			case GPU_LA8:
			case GPU_HILO8:
				srcBuf16 = (u16 *)srcBuf;
				for (idx = 0; idx < wSource * hSource; idx++) {
					((u16 *)srcConverted)[idx] = (((srcBuf16[idx] << 8) & 0xff00) |
					                              ((srcBuf16[idx] >> 8) & 0x00ff));
				}
				break;
			case GPU_RGB565:
			case GPU_RGBA5551:
			case GPU_RGBA4:
				for (idx = 0; idx < wSource * hSource; idx++)
					((u16 *)srcConverted)[idx] = ((u16 *)srcBuf)[idx];
				break;
			case GPU_A8:
			case GPU_L8:
			case GPU_LA4:
				for (idx = 0; idx < wSource * hSource; idx++)
					srcConverted[idx] = ((u8 *)srcBuf)[idx];
				break;
			case GPU_L4:
			case GPU_A4:
				for (idx = 0; idx < wSource * hSource / 2; idx++)
					srcConverted[idx] = ((u8 *)srcBuf)[idx];
				break;
			case GPU_ETC1:
			case GPU_ETC1A4:
				linearFree(srcConverted);
				return;
		}
	} else
		srcConverted = (u8 *)srcBuf;

	for (y = 0; y < copyH; y++) {
		u32 yOffSrc = (ySrc + y) * hSource / hSrc;
		u32 yOffSrcCoarse = yOffSrc & ~7;

		u32 yOffDst = (yFlip) ? ((hSrc - y - 1) + yDst) : (y + yDst);
		u32 yOffDstCoarse = yOffDst & ~7;

		for (x = 0; x < copyW; x++) {
			u32 xOffDst = x + xDst;
			u32 offset = mortonOffset(xOffDst, yOffDst) + yOffDstCoarse * wDst;

			u32 xOffSrc = (xSrc + x) * wSource / wSrc;
			u32 locate = (isBlockSrc) ? (mortonOffset(xOffSrc, yOffSrc) + yOffSrcCoarse * wSrc)
			                          : (xOffSrc + yOffSrc * wSource);

			if (offset >= wDst * hDst) continue;
			if (locate >= (u32)(wSource * hSource)) continue;

			u8 *dstByte, *srcByte;

			if (halfByte) {
				srcByte = &srcConverted[(locate * pixSize) >> 1];
				dstByte = &((u8 *)dstBuf)[(offset * pixSize) >> 1];
				if (x & 1)
					*dstByte = (*srcByte & 0xf0) | (*dstByte & 0x0f);
				else
					*dstByte = (*dstByte & 0xf0) | (*srcByte & 0x0f);
			} else {
				srcByte = &srcConverted[locate * pixSize];
				dstByte = &((u8 *)dstBuf)[offset * pixSize];
				for (int i = 0; i < (int)pixSize; i++)
					*dstByte++ = *srcByte++;
			}
		}
	}

	if (!isBlockSrc)
		linearFree((void *)srcConverted);

	return;
}

void N3DContext::opDataToBlockTex(u32 *srcBuf, u32 *dstBuf, int xSource, int ySource,
                                  int wSource, int hSource, int wDest, int hDest,
                                  GPU_TEXCOLOR format, int scale, bool isBlockSrc) {
	opArbDataToArbBlockTexOffset(srcBuf,  dstBuf,  wSource, hSource,
	                             xSource, ySource, wSource, hSource,
	                             0,       0,       wDest,   hDest,
	                             format, scale, isBlockSrc);
}

} // end namespace N3DS_3D