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

#if defined(__3DS__)
#include "graphics/3ds/n3d.h"

#include "common/rect.h"
#include "common/textconsole.h"

#include "graphics/surface.h"

#include "math/glmath.h"
#include "math/vector2d.h"
#include "math/rect2d.h"
#include "math/quat.h"

#include "engines/playground3d/gfx.h"
#include "engines/playground3d/gfx_n3ds.h"

namespace Playground3d {

static const float triOffsetVertices[] =
{
	//  X      Y     Z
	// 1st triangle
	-1.0f,  1.0f, 0.0f,
	 1.0f,  1.0f, 0.0f,
	 0.0f, -1.0f, 0.0f,
	// 2nd triangle
	-0.5f,  0.5f, 0.0f,
	 0.5f,  0.5f, 0.0f,
	 0.0f, -0.5f, 0.0f,
};

static const float dimRegionVertices[] = {
	//  X      Y
	-0.5f,  0.5f,
	 0.5f,  0.5f,
	-0.5f, -0.5f,
	 0.5f, -0.5f,
};

// The next two arrays ~could~ be combined into a single
//	array like triOffsetVertices[]. I'm choosing not to,
//	however, in order to show what can be done when that
//	isn't possible.

// 1st viewport test box
static const float boxVertices[] = {
	//  X      Y
	-1.0f,  1.0f,
	 1.0f,  1.0f,
	-1.0f, -1.0f,
	 1.0f, -1.0f,
};

// 2nd viewport test box
static const float boxVertices2[] = {
	//  X      Y
	-0.1f,  0.1f,
	 0.1f,  0.1f,
	-0.1f, -0.1f,
	 0.1f, -0.1f,
};

// Account for the fact that C3D_Tex objects must have
//	lengths and widths that are powers of two.
// Swap T coordinates to account for the fact that
//	3DS textures are upside-down.
static const float bitmapVertices[] = {
	//  X      Y     S         T
	-0.2f,  0.2f, 0.0f,        120.f/128.f,
	 0.2f,  0.2f, 120.f/128.f, 120.f/128.f,
	-0.2f, -0.2f, 0.0f,        0.0f,
	 0.2f, -0.2f, 120.f/128.f, 0.0f,
};

template<class T>
static T nextHigher2(T k) {
	if (k == 0)
		return 1;
	--k;

	for (uint i = 1; i < sizeof(T) * 8; i <<= 1)
		k = k | k >> i;

	return k + 1;
}

Renderer *CreateGfxN3DS(OSystem *system) {
	return new N3DSRenderer(system);
}

N3DSRenderer::N3DSRenderer(OSystem *system) :
		Renderer(system),
		_currentViewport(kOriginalWidth, kOriginalHeight),
		_cubeShader(nullptr),
		_offsetShader(nullptr),
		_fadeShader(nullptr),
		_viewportShader(nullptr),
		_bitmapShader(nullptr) {
	// Create context that corresponds to the Citro3D setting of the 3DS backend.
	_backendContext = N3DS_3D::createContext();
	// Create context that corresponds to Open GL settings to be used for Playground3D engine rendering.
	_p3dContext = N3DS_3D::createOGLContext();

	_gameScreenTex = N3D_GetGameScreen();
	_gameScreenTarget = N3D_C3D_RenderTargetCreateFromTex(_gameScreenTex, GPU_TEXFACE_2D, 0, GPU_RB_DEPTH16);

	N3D_C3D_TexEnvInit(&_p3dTexEnv);
	N3D_C3D_TexEnvFunc(&_p3dTexEnv, C3D_Both, GPU_REPLACE);
	N3D_C3D_TexEnvOpRgb(&_p3dTexEnv, GPU_TEVOP_RGB_SRC_COLOR/*, GPU_TEVOP_RGB_SRC_COLOR, GPU_TEVOP_RGB_SRC_COLOR*/);
	N3D_C3D_TexEnvOpAlpha(&_p3dTexEnv, GPU_TEVOP_A_SRC_ALPHA/*, GPU_TEVOP_A_SRC_ALPHA, GPU_TEVOP_A_SRC_ALPHA*/);
}

N3DSRenderer::~N3DSRenderer() {
	linearFree(_cubeVBO);
	linearFree(_offsetVBO);
	linearFree(_fadeVBO);
	linearFree(_viewportVBO_1);
	linearFree(_viewportVBO_2);
	linearFree(_bitmapVBO);

	delete _cubeShader;
	delete _offsetShader;
	delete _fadeShader;
	delete _viewportShader;
	delete _bitmapShader;

	N3D_UnloadShaderProgram("playground3d_cube");
	N3D_UnloadShaderProgram("playground3d_offset");
	N3D_UnloadShaderProgram("playground3d_fade");
	N3D_UnloadShaderProgram("playground3d_viewport");
	N3D_UnloadShaderProgram("playground3d_bitmap");

	N3D_C3D_RenderTargetDelete(_gameScreenTarget);

	N3DS_3D::setContext(_backendContext);
	N3DS_3D::destroyContext(_p3dContext);
	N3DS_3D::destroyContext(_backendContext);
	N3DS_3D::destroyNative3D();
}

void N3DSRenderer::init() {
	debug("Initializing Nintendo 3DS renderer");

	computeScreenViewport();

	_cubeProgram = N3D_LoadShaderProgram("playground3d_cube", &_cubeProgramFlags);
	_cubeShader = new N3DS_3D::ShaderObj(_cubeProgram, _cubeProgramFlags);
	_cubeVBO = N3D_CreateBuffer(sizeof(float) * 11 * 24, cubeVertices, 0x4);
	_cubeShader->addAttrLoader(0, GPU_FLOAT, 2);
	_cubeShader->addAttrLoader(1, GPU_FLOAT, 3);
	_cubeShader->addAttrLoader(2, GPU_FLOAT, 3);
	_cubeShader->addAttrLoader(3, GPU_FLOAT, 3);
	_cubeShader->addBufInfo(_cubeVBO, 11 * sizeof(float), 4, 0x3210);

	_offsetProgram = N3D_LoadShaderProgram("playground3d_offset", &_offsetProgramFlags);
	_offsetShader = new N3DS_3D::ShaderObj(_offsetProgram, _offsetProgramFlags);
	_offsetVBO = N3D_CreateBuffer(sizeof(float) * 3 * 6, triOffsetVertices, 0x4);
	_offsetShader->addAttrLoader(0, GPU_FLOAT, 3);
	_offsetShader->addBufInfo(_offsetVBO, 3 * sizeof(float), 1, 0x0);

	_fadeProgram = N3D_LoadShaderProgram("playground3d_fade", &_fadeProgramFlags);
	_fadeShader = new N3DS_3D::ShaderObj(_fadeProgram, _fadeProgramFlags);
	_fadeVBO = N3D_CreateBuffer(sizeof(float) * 2 * 4, dimRegionVertices, 0x4);
	_fadeShader->addAttrLoader(0, GPU_FLOAT, 2);
	_fadeShader->addBufInfo(_fadeVBO, 2 * sizeof(float), 1, 0x0);

	// DO THIS AS A CLONE OF _fadeProgram?
	_viewportProgram = N3D_LoadShaderProgram("playground3d_viewport", &_viewportProgramFlags);
	_viewportShader = new N3DS_3D::ShaderObj(_viewportProgram, _viewportProgramFlags);
	_viewportVBO_1 = N3D_CreateBuffer(sizeof(float) * 2 * 4, boxVertices, 0x4);
	_viewportVBO_2 = N3D_CreateBuffer(sizeof(float) * 2 * 4, boxVertices2, 0x4);
	_viewportShader->addAttrLoader(0, GPU_FLOAT, 2);
	_viewportShader->addBufInfo(_viewportVBO_1, 2 * sizeof(float), 1, 0x0);
	N3D_BufInfo_Init(&_vpBuf2);
	N3D_BufInfo_Add(&_vpBuf2, _viewportVBO_2, 2 * sizeof(float), 1, 0x0);

	_bitmapProgram = N3D_LoadShaderProgram("playground3d_bitmap", &_bitmapProgramFlags);
	_bitmapShader = new N3DS_3D::ShaderObj(_bitmapProgram, _bitmapProgramFlags);
	_bitmapVBO = N3D_CreateBuffer(sizeof(float) * 4 * 4, bitmapVertices, 0x4);
	_bitmapShader->addAttrLoader(0, GPU_FLOAT, 2);
	_bitmapShader->addAttrLoader(1, GPU_FLOAT, 2);
	_bitmapShader->addBufInfo(_bitmapVBO, 4 * sizeof(float), 2, 0x10);
}

void N3DSRenderer::deinit() {
	N3D_C3D_TexDelete(&_textureRgba);
	N3D_C3D_TexDelete(&_textureRgb);
	N3D_C3D_TexDelete(&_textureRgb565);
	N3D_C3D_TexDelete(&_textureRgba5551);
	N3D_C3D_TexDelete(&_textureRgba4444);
}

#define FLOAT_TO_8BIT(fnum)		((u32)(0.5f + (fnum) * (float)255))

void N3DSRenderer::clear(const Math::Vector4d &clearColor) {
	N3DS_3D::setContext(_p3dContext);
	N3D_C3D_FrameBegin(0);
	N3D_C3D_FrameDrawOn(_gameScreenTarget);
	u32 clearcolorint = FLOAT_TO_8BIT(clearColor.x()) << 24
	                  | FLOAT_TO_8BIT(clearColor.y()) << 16
	                  | FLOAT_TO_8BIT(clearColor.z()) << 8
	                  | FLOAT_TO_8BIT(clearColor.w());
	N3D_C3D_RenderTargetClear(_gameScreenTarget, C3D_CLEAR_ALL, clearcolorint, 0xFFFF);
}

#undef FLOAT_TO_8BIT

void N3DSRenderer::setupViewport(int x, int y, int width, int height) {
	N3D_C3D_SetViewport(x, y, width, height);
}

void N3DSRenderer::flipBuffer() {
	N3D_C3D_FrameEnd(0);
	N3D_C3D_TexBind(0, NULL);
	N3DS_3D::setContext(_backendContext);
}

void N3DSRenderer::enableFog(const Math::Vector4d &fogColor) {
	// TODO
}

void N3DSRenderer::loadTextureRGBA(Graphics::Surface *texture) {
	// texture->w and texture->h are both 120
	int wPow2 = nextHigher2(texture->w); // 128
	int hPow2 = nextHigher2(texture->h); // 128
	int pitchPow2 = wPow2 * 4;
	N3D_C3D_TexInit(&_textureRgba, (u16)wPow2, (u16)hPow2, GPU_RGBA8);
	N3D_C3D_TexSetFilter(&_textureRgba, GPU_NEAREST, GPU_NEAREST);

	// Only create the temporary buffer AFTER initializing the C3D_Tex,
	//	so as to prevent fragmentation of linear memory.
	void *tmp = N3D_CreateBuffer(wPow2 * hPow2 * 4);

	// Copy texture rows into temporary buffer.
	void *texPtr, *tmpPtr;
	for (int i = 0; i < texture->h; i++) {
		texPtr = texture->getBasePtr(0, i);
		tmpPtr = (byte *)tmp + i * pitchPow2;
		memcpy(tmpPtr, texPtr, texture->w * 4);
	}

	// Swizzle temporary buffer into C3D_Tex
	// N3D_DataToBlockTex and N3D_ArbDataToArbBlockTexOffset will swap bytes to
	//	little endian order (as required for the RGBA8 format for 3DS textures)
	//	before swizzling. Input pixels are assumed to be big endian.
	N3D_DataToBlockTex((u32 *)tmp, (u32 *)_textureRgba.data, 0, 0, wPow2, hPow2, wPow2, hPow2, GPU_RGBA8, false);

	// Flush the texture changes.
	N3D_C3D_TexFlush(&_textureRgba);
	// Delete the temporary buffer.
	N3D_FreeBuffer(tmp);
}

void N3DSRenderer::loadTextureRGB(Graphics::Surface *texture) {
	// texture->w and texture->h are both 120
	int wPow2 = nextHigher2(texture->w); // 128
	int hPow2 = nextHigher2(texture->h); // 128
	// Make an RGBA8 texture since we need all 4 components
	N3D_C3D_TexInit(&_textureRgb, (u16)wPow2, (u16)hPow2, GPU_RGBA8);
	N3D_C3D_TexSetFilter(&_textureRgb, GPU_NEAREST, GPU_NEAREST);

	// Only create the temporary buffer AFTER initializing the C3D_Tex,
	//	so as to prevent fragmentation of linear memory.
	void *tmp = N3D_CreateBuffer(wPow2 * texture->h * 4);

	// Copy RGB8 pixels into temporary buffer while making them visible.
	byte *texPtr, *tmpPtr;
	for (int y = 0; y < texture->h; y++) {
		for (int x = 0; x < texture->w; x++) {
			texPtr = (byte *)texture->getBasePtr(x, y);
			tmpPtr = (byte *)tmp + (y * wPow2 * 4) + (x * 4);
			memcpy(tmpPtr, texPtr, texture->format.bytesPerPixel);
			tmpPtr[3] = 0xff;
		}
	}

	// Swizzle temporary buffer into C3D_Tex
	// N3D_DataToBlockTex and N3D_ArbDataToArbBlockTexOffset will swap bytes to
	//	little endian order (as required for the RGBA8 format for 3DS textures)
	//	before swizzling. Input pixels are assumed to be big endian.
	N3D_DataToBlockTex((u32 *)tmp, (u32 *)_textureRgb.data, 0, 0, wPow2, hPow2, wPow2, hPow2, GPU_RGBA8, false);

	// Flush the texture changes.
	N3D_C3D_TexFlush(&_textureRgb);
	// Delete the temporary buffer.
	N3D_FreeBuffer(tmp);
}

void N3DSRenderer::loadTextureRGB565(Graphics::Surface *texture) {
	// texture->w and texture->h are both 120
	int wPow2 = nextHigher2(texture->w); // 128
	int hPow2 = nextHigher2(texture->h); // 128
	// Make an RGBA8 texture since we need all 4 components
	N3D_C3D_TexInit(&_textureRgb565, (u16)wPow2, (u16)hPow2, GPU_RGBA8);
	N3D_C3D_TexSetFilter(&_textureRgb565, GPU_NEAREST, GPU_NEAREST);

	// Only create the temporary buffer AFTER initializing the C3D_Tex,
	//	so as to prevent fragmentation of linear memory.
	void *tmp = N3D_CreateBuffer(wPow2 * texture->h * 4);

	// Convert RGB565 pixels into RGBA8 pixels.
	u16 *texPtr;
	byte *tmpPtr;
	for (int y = 0; y < texture->h; y++) {
		for (int x = 0; x < texture->w; x++) {
			texPtr = (u16 *)texture->getBasePtr(x, y);
			tmpPtr = (byte *)tmp + (y * wPow2 * 4) + (x * 4);
			tmpPtr[0] = ((*texPtr & 0xF800) >> 11) / 31 * 255;	// R
			tmpPtr[1] = ((*texPtr & 0x07E0) >>  5) / 63 * 255;	// G
			tmpPtr[2] = ((*texPtr & 0x001F) >>  0) / 31 * 255;	// B
			tmpPtr[3] = 0xff;									// A
		}
	}

	// Swizzle temporary buffer into C3D_Tex
	// N3D_DataToBlockTex and N3D_ArbDataToArbBlockTexOffset will swap bytes to
	//	little endian order (as required for the RGBA8 format for 3DS textures)
	//	before swizzling. Input pixels are assumed to be big endian.
	N3D_DataToBlockTex((u32 *)tmp, (u32 *)_textureRgb565.data, 0, 0, wPow2, hPow2, wPow2, hPow2, GPU_RGBA8, false);

	// Flush the texture changes.
	N3D_C3D_TexFlush(&_textureRgb565);
	// Delete the temporary buffer.
	N3D_FreeBuffer(tmp);
}

void N3DSRenderer::loadTextureRGBA5551(Graphics::Surface *texture) {
	// For the sake of example, we will use GSPGPU_FlushDataCache and
	//	N3D_C3D_SyncDisplayTransfer instead of N3D_DataToBlockTex.

	// texture->w and texture->h are both 120
	int wPow2 = nextHigher2(texture->w); // 128
	int hPow2 = nextHigher2(texture->h); // 128
	// Make an RGBA8 texture since we need all 4 components
	N3D_C3D_TexInit(&_textureRgba5551, (u16)wPow2, (u16)hPow2, GPU_RGBA8);
	N3D_C3D_TexSetFilter(&_textureRgba5551, GPU_NEAREST, GPU_NEAREST);

	// Only create the temporary buffer AFTER initializing the C3D_Tex,
	//	so as to prevent fragmentation of linear memory.
	void *tmp = N3D_CreateBuffer(wPow2 * texture->h * 4);

	// Convert pixels to little endian RGBA8, as N3D_C3D_SyncDisplayTransfer
	//	does not do any byte swapping.
	u16 *texPtr;
	byte *tmpPtr;
	for (int y = 0; y < texture->h; y++) {
		for (int x = 0; x < texture->w; x++) {
			texPtr = (u16 *)texture->getBasePtr(x, y);
			tmpPtr = (byte *)tmp + (y * wPow2 * 4) + (x * 4);
			tmpPtr[0] = ((*texPtr & 0x0001) >>  0) * 255;		// A
			tmpPtr[1] = ((*texPtr & 0x003E) >>  1) / 31 * 255;	// B
			tmpPtr[2] = ((*texPtr & 0x07C0) >>  6) / 31 * 255;	// G
			tmpPtr[3] = ((*texPtr & 0xF800) >> 11) / 31 * 255;	// R
		}
	}

	// Swizzle temporary buffer into C3D_Tex
	// N3D_C3D_SyncDisplayTransfer will also swizzle data (and also unswizzle it).
	//	However, the input pixels must already be in RGBA8 little endian order, and
	//	GSPGPU_FlushDataCache must be called on the input pixels beforehand.
	GSPGPU_FlushDataCache(tmp, wPow2 * texture->h * texture->format.bytesPerPixel);
	N3D_C3D_SyncDisplayTransfer((u32 *)tmp, GX_BUFFER_DIM(wPow2, texture->h), (u32 *)_textureRgba5551.data, GX_BUFFER_DIM(wPow2, hPow2),
	                            (GX_TRANSFER_FLIP_VERT(1) | GX_TRANSFER_OUT_TILED(1) | GX_TRANSFER_RAW_COPY(0) |
	                             GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGBA8) |
	                             GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO)));

	// Flush the texture changes.
	N3D_C3D_TexFlush(&_textureRgba5551);
	// Delete the temporary buffer.
	N3D_FreeBuffer(tmp);
}

void N3DSRenderer::loadTextureRGBA4444(Graphics::Surface *texture) {
	// For the sake of example, we will use GSPGPU_FlushDataCache and
	//	N3D_C3D_SyncDisplayTransfer instead of N3D_DataToBlockTex.

	// texture->w and texture->h are both 120
	int wPow2 = nextHigher2(texture->w); // 128
	int hPow2 = nextHigher2(texture->h); // 128
	// Make an RGBA8 texture since we need all 4 components
	N3D_C3D_TexInit(&_textureRgba4444, (u16)wPow2, (u16)hPow2, GPU_RGBA8);
	N3D_C3D_TexSetFilter(&_textureRgba4444, GPU_NEAREST, GPU_NEAREST);

	// Only create the temporary buffer AFTER initializing the C3D_Tex,
	//	so as to prevent fragmentation of linear memory.
	void *tmp = N3D_CreateBuffer(wPow2 * texture->h * 4);

	// Convert pixels to little endian RGBA8, as N3D_C3D_SyncDisplayTransfer
	//	does not do any byte swapping.
	u16 *texPtr;
	byte *tmpPtr;
	for (int y = 0; y < texture->h; y++) {
		for (int x = 0; x < texture->w; x++) {
			texPtr = (u16 *)texture->getBasePtr(x, y);
			tmpPtr = (byte *)tmp + (y * wPow2 * 4) + (x * 4);
			tmpPtr[0] = ((*texPtr & 0x000F) >>  0) / 15 * 255;	// A
			tmpPtr[1] = ((*texPtr & 0x00F0) >>  4) / 15 * 255;	// B
			tmpPtr[2] = ((*texPtr & 0x0F00) >>  8) / 15 * 255;	// G
			tmpPtr[3] = ((*texPtr & 0xF000) >> 12) / 15 * 255;	// R
		}
	}

	// Swizzle temporary buffer into C3D_Tex
	// N3D_C3D_SyncDisplayTransfer will also swizzle data (and also unswizzle it).
	//	However, the input pixels must already be in RGBA8 little endian order, and
	//	GSPGPU_FlushDataCache must be called on the input pixels beforehand.
	GSPGPU_FlushDataCache(tmp, wPow2 * texture->h * texture->format.bytesPerPixel);
	N3D_C3D_SyncDisplayTransfer((u32 *)tmp, GX_BUFFER_DIM(wPow2, texture->h), (u32 *)_textureRgba4444.data, GX_BUFFER_DIM(wPow2, hPow2),
	                            (GX_TRANSFER_FLIP_VERT(1) | GX_TRANSFER_OUT_TILED(1) | GX_TRANSFER_RAW_COPY(0) |
	                             GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGBA8) |
	                             GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO)));

	// Flush the texture changes.
	N3D_C3D_TexFlush(&_textureRgba4444);
	// Delete the temporary buffer.
	N3D_FreeBuffer(tmp);
}

void N3DSRenderer::drawCube(const Math::Vector3d &pos, const Math::Vector3d &roll) {
	N3D_C3D_TexEnvSrc(&_p3dTexEnv, C3D_Both, GPU_PRIMARY_COLOR/*, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR*/);
	N3D_C3D_SetTexEnv(0, &_p3dTexEnv);
	N3D_DepthTestEnabled(true);
	N3D_DepthMask(true);
	N3D_BlendEnabled(false);
	N3D_BlendFunc(GPU_ONE, GPU_ZERO);

	auto rotateMatrix = (Math::Quaternion::fromEuler(roll.x(), roll.y(), roll.z(), Math::EO_XYZ)).inverse().toMatrix();

	N3DS_3D::getActiveContext()->changeShader(_cubeShader);

	_cubeShader->setUniform("mvpMatrix", GPU_VERTEX_SHADER, _mvpMatrix);
	_cubeShader->setUniform("rotateMatrix", GPU_VERTEX_SHADER, rotateMatrix);
	_cubeShader->setUniform("modelPos", GPU_VERTEX_SHADER, pos);

	N3D_C3D_DrawArrays(GPU_TRIANGLE_STRIP, 0, 4);
	N3D_C3D_DrawArrays(GPU_TRIANGLE_STRIP, 4, 4);
	N3D_C3D_DrawArrays(GPU_TRIANGLE_STRIP, 8, 4);
	N3D_C3D_DrawArrays(GPU_TRIANGLE_STRIP, 12, 4);
	N3D_C3D_DrawArrays(GPU_TRIANGLE_STRIP, 16, 4);
	N3D_C3D_DrawArrays(GPU_TRIANGLE_STRIP, 20, 4);

}

void N3DSRenderer::drawPolyOffsetTest(const Math::Vector3d &pos, const Math::Vector3d &roll) {
	N3D_C3D_TexEnvSrc(&_p3dTexEnv, C3D_Both, GPU_PRIMARY_COLOR/*, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR*/);
	N3D_C3D_SetTexEnv(0, &_p3dTexEnv);
	N3D_DepthTestEnabled(true);
	N3D_DepthMask(true);
	N3D_BlendEnabled(false);
	N3D_BlendFunc(GPU_ONE, GPU_ZERO);

	auto rotateMatrix = (Math::Quaternion::fromEuler(roll.x(), roll.y(), roll.z(), Math::EO_XYZ)).inverse().toMatrix();
	Math::Vector3d colorVec = Math::Vector3d(0.0f, 1.0f, 0.0f);

	N3DS_3D::getActiveContext()->changeShader(_offsetShader);

	_offsetShader->setUniform("mvpMatrix", GPU_VERTEX_SHADER, _mvpMatrix);
	_offsetShader->setUniform("rotateMatrix", GPU_VERTEX_SHADER, rotateMatrix);
	_offsetShader->setUniform("modelPos", GPU_VERTEX_SHADER, pos);
	_offsetShader->setUniform("renderColor", GPU_VERTEX_SHADER, colorVec);
	N3D_C3D_DrawArrays(GPU_TRIANGLES, 0, 3);

	N3D_PolygonOffsetEnabled(true);
	// Positive offset -> subsequent polygons will be pushed back
	// Negative offset -> subsequent polygons will be pulled forward
	N3D_PolygonOffset(-1000.f);

	colorVec = Math::Vector3d(1.0f, 1.0f, 1.0f);
	_offsetShader->setUniform("renderColor", GPU_VERTEX_SHADER, colorVec);
	N3D_C3D_DrawArrays(GPU_TRIANGLES, 3, 3);

	N3D_PolygonOffsetEnabled(false);
}

void N3DSRenderer::dimRegionInOut(float fade) {
	N3D_C3D_TexEnvSrc(&_p3dTexEnv, C3D_Both, GPU_PRIMARY_COLOR/*, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR*/);
	N3D_C3D_SetTexEnv(0, &_p3dTexEnv);
	N3D_DepthTestEnabled(false);
	N3D_DepthMask(false);
	N3D_BlendEnabled(true);
	N3D_BlendFunc(GPU_ONE, GPU_ONE_MINUS_SRC_ALPHA);

	N3DS_3D::getActiveContext()->changeShader(_fadeShader);
	_fadeShader->setUniform("alphaLevel", GPU_VERTEX_SHADER, 1.0f - fade);
	N3D_C3D_DrawArrays(GPU_TRIANGLE_STRIP, 0, 4);
}

void N3DSRenderer::drawInViewport() {
	N3D_C3D_TexEnvSrc(&_p3dTexEnv, C3D_Both, GPU_PRIMARY_COLOR/*, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR*/);
	N3D_C3D_SetTexEnv(0, &_p3dTexEnv);
	N3D_DepthTestEnabled(false);
	N3D_DepthMask(false);
	N3D_BlendEnabled(true);
	N3D_BlendFunc(GPU_ONE, GPU_ONE_MINUS_SRC_ALPHA);

	Math::Vector3d colorVec = Math::Vector3d(0.0f, 1.0f, 0.0f);

	N3DS_3D::getActiveContext()->changeShader(_viewportShader);
	_viewportShader->setUniform("offsetXY", GPU_VERTEX_SHADER, 0.0f, 0.0f);
	_viewportShader->setUniform("renderColor", GPU_VERTEX_SHADER, colorVec);
	N3D_C3D_DrawArrays(GPU_TRIANGLE_STRIP, 0, 4);

	_drawViewportPos.x() += 0.01f;
	_drawViewportPos.y() += 0.01f;
	// Ensure red square is fully out of view before moving
	//	it to the opposite corner.
	if (_drawViewportPos.x() >= 1.1f) {
		_drawViewportPos.x() = -1.1f;
		_drawViewportPos.y() = -1.1f;
	}
	_drawViewportPos.z() = 0.0f;

	colorVec = Math::Vector3d(1.0f, 0.0f, 0.0f);
	N3D_C3D_SetBufInfo(&_vpBuf2);
	N3D_PolygonOffsetEnabled(true);
	// Positive offset -> subsequent polygons will be pushed back
	// Negative offset -> subsequent polygons will be pulled forward
	N3D_PolygonOffset(-1000.f);
	_viewportShader->setUniform("offsetXY", GPU_VERTEX_SHADER, _drawViewportPos);
	_viewportShader->setUniform("renderColor", GPU_VERTEX_SHADER, colorVec);
	N3D_C3D_DrawArrays(GPU_TRIANGLE_STRIP, 0, 4);
	N3D_PolygonOffsetEnabled(false);
}

void N3DSRenderer::drawRgbaTexture() {
	Math::Vector2d offset;
	N3D_C3D_TexEnvSrc(&_p3dTexEnv, C3D_Both, GPU_TEXTURE0/*, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR*/);
	N3D_C3D_SetTexEnv(0, &_p3dTexEnv);
	N3D_DepthTestEnabled(false);
	N3D_DepthMask(false);
	N3D_BlendEnabled(true);
	N3D_BlendFunc(GPU_ONE, GPU_ONE_MINUS_SRC_ALPHA);

	N3DS_3D::getActiveContext()->changeShader(_bitmapShader);

	offset.setX(-0.8f);
	offset.setY(0.8f);
	_bitmapShader->setUniform("offsetXY", GPU_VERTEX_SHADER, offset);
	N3D_C3D_TexBind(0, &_textureRgba);
	N3D_C3D_DrawArrays(GPU_TRIANGLE_STRIP, 0, 4);

	offset.setX(-0.3f);
	offset.setY(0.8f);
	_bitmapShader->setUniform("offsetXY", GPU_VERTEX_SHADER, offset);
	N3D_C3D_TexBind(0, &_textureRgb);
	N3D_C3D_DrawArrays(GPU_TRIANGLE_STRIP, 0, 4);

	offset.setX(0.2f);
	offset.setY(0.8f);
	_bitmapShader->setUniform("offsetXY", GPU_VERTEX_SHADER, offset);
	N3D_C3D_TexBind(0, &_textureRgb565);
	N3D_C3D_DrawArrays(GPU_TRIANGLE_STRIP, 0, 4);

	offset.setX(0.7f);
	offset.setY(0.8f);
	_bitmapShader->setUniform("offsetXY", GPU_VERTEX_SHADER, offset);
	N3D_C3D_TexBind(0, &_textureRgba5551);
	N3D_C3D_DrawArrays(GPU_TRIANGLE_STRIP, 0, 4);

	offset.setX(-0.8f);
	offset.setY(0.2f);
	_bitmapShader->setUniform("offsetXY", GPU_VERTEX_SHADER, offset);
	N3D_C3D_TexBind(0, &_textureRgba4444);
	N3D_C3D_DrawArrays(GPU_TRIANGLE_STRIP, 0, 4);
}

} // End of namespace Playground3d

#endif // defined(__3DS__)
