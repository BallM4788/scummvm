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
	N3DS::OSystem_3DS *gsys3DS = dynamic_cast<N3DS::OSystem_3DS *>(g_system);
	return gsys3DS->getGameSurface();
}

void *N3DContext::opCreateBuffer(size_t size, const void *data, size_t alignment) {
	N3DS::OSystem_3DS *gsys3DS = dynamic_cast<N3DS::OSystem_3DS *>(g_system);
	return gsys3DS->createBuffer(size, data, alignment);
}

void N3DContext::opFreeBuffer(void *linearBuffer) {
	N3DS::OSystem_3DS *gsys3DS = dynamic_cast<N3DS::OSystem_3DS *>(g_system);
	return gsys3DS->freeBuffer(linearBuffer);
}

static int getPixelSizeInBytes(GPU_TEXCOLOR format) {
	if (format == GPU_RGBA8) return 4;
	if (format == GPU_RGB8) return 3;
	// RGB565, RGBA5551, RGBA4, LA8, HILO8
	if ((format >= GPU_RGB565) && (format <= GPU_HILO8)) return 2;
	// A8, L8, LA4, L4, A4
	if ((format >= GPU_LA8) && (format <= GPU_LA4)) return 1;
	// ETC1, ETC1A4 (we don't wanna use these)
	return 0;
}

alignas(32) static const u32 mortonLUT[8][8] = {
	{ 0,  1,  4,  5, 16, 17, 20, 21 },
	{ 2,  3,  6,  7, 18, 19, 22, 23 },
	{ 8,  9, 12, 13, 24, 25, 28, 29 },
	{10, 11, 14, 15, 26, 27, 30, 31 },
	{32, 33, 36, 37, 48, 49, 52, 53 },
	{34, 35, 38, 39, 50, 51, 54, 55 },
	{40, 41, 44, 45, 56, 57, 60, 61 },
	{42, 43, 46, 47, 58, 59, 62, 63 }
};

static inline u32 getMortonOffset(u32 x, u32 y) {
	return mortonLUT[y & 7][x & 7] + ((x & ~7) << 3);
}

__attribute__((optimize("-O3")))
void N3DContext::opDataToBlockTex(u32 *srcBuf, const u32 xSrc, const u32 ySrc, const u32 wSrc, const u32 hSrc,
                                  u32 *dstBuf, const u32 xDst, const u32 yDst, const u32 wDst, const u32 hDst,
                                  const u32 copyWidth, const u32 copyHeight, GPU_TEXCOLOR format,
                                  bool reorderComponents, bool isBlockSrc) {

	const u32 pixSize = getPixelSizeInBytes(format);
	// Textures are flipped vertically in 3DS block format.
	const bool isYFlipped   = !isBlockSrc;
	// L4 and A4 texels are half a byte each, not a full byte.
	const bool halfByte = ((format == GPU_L4) || (format == GPU_A4));
	// Texel components in block textures are already in the correct order.
	const bool swapBytes = (reorderComponents && !isBlockSrc);
	// Of the supported 16-bit formats, only LA8 and HILO8 need their color
	//	components reordered.
	const bool swappable16 = (format == GPU_LA8) || (format == GPU_HILO8);

	// u8 pointers to srcBuf and dstBuf.
	u8*  src8 =  (u8  *)srcBuf;
	u8*  dst8 =  (u8  *)dstBuf;
	// u16 pointers to srcBuf and dstBuf.
	u16* src16 = (u16 *)srcBuf;
	u16* dst16 = (u16 *)dstBuf;

	// Final indices in srcBuf and dstBuf.
	const u32 maxSrcIndex = (wSrc * hSrc) - 1;
	const u32 maxDstIndex = (wDst * hDst) - 1;

	for (u32 y = 0; y < copyHeight; y++) {
		// In block order textures, the image is flipped vertically.
		u32 yOffSrc = isYFlipped ?             (ySrc + y)
		                         : (hSrc - 1 - (ySrc + y));
		u32 yOffDst =               hDst - 1 - (yDst + y);

		// Linear index at which the row of 8x8 tiles containing row yOffSrc begins. This
		//	is only used if 'isBlockSrc' is true.
		u32 yOffSrcCoarseRow = (yOffSrc & ~7) * wSrc;
		// Linear index at which the row of 8x8 tiles containing row yOffDst begins.
		u32 yOffDstCoarseRow = (yOffDst & ~7) * wDst;

		for (u32 x = 0; x < copyWidth; x++) {
			u32 xOffSrc = xSrc + x;
			// Compute array index of source texel at linear coordinate (xOffSrc, yOffSrc).
			u32 srcIndex = (!isBlockSrc) ? (xOffSrc + yOffSrc * wSrc)
			                             : (getMortonOffset(xOffSrc, yOffSrc) + yOffSrcCoarseRow);
			if (srcIndex > maxSrcIndex) continue;

			u32 xOffDst = xDst + x;
			// Compute array index of destination texel at linear coordinate (xOffDst, yOffDst).
			u32 dstIndex = getMortonOffset(xOffDst, yOffDst) + yOffDstCoarseRow;
			if (dstIndex > maxDstIndex) continue;

			if (!halfByte) {
				switch (pixSize) {
					case 4:
						if (swapBytes)
							dstBuf[dstIndex] = SWAP_BYTES_32(srcBuf[srcIndex]);
						else
							dstBuf[dstIndex] =               srcBuf[srcIndex];
						break;
					case 2:
						if (swapBytes && swappable16)
							dst16[dstIndex] = SWAP_BYTES_16(src16[srcIndex]);
						else
							dst16[dstIndex] =               src16[srcIndex];
						break;
					default: {
						u8 *srcByte = &src8[srcIndex * pixSize];
						u8 *dstByte = &dst8[dstIndex * pixSize];
						for (u32 i = 0; i < pixSize; i++) {
							dstByte[i] = swapBytes ? srcByte[pixSize - 1 - i] : srcByte[i];
						}
					}
				}
			} else {
				u8  srcByte =  src8[srcIndex >> 1];
				u8 *dstByte = &dst8[dstIndex >> 1];
				if (x & 1)
					*dstByte = ( srcByte & 0xf0) | (*dstByte & 0x0f);
				else
					*dstByte = (*dstByte & 0xf0) | ( srcByte & 0x0f);
			}
		}
	}
}

} // end namespace N3DS_3D
