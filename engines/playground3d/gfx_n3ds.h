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

#ifndef PLAYGROUND3D_GFX_N3DS_H
#define PLAYGROUND3D_GFX_N3DS_H

#include "common/rect.h"

#include "math/rect2d.h"

#include "graphics/3ds/n3d.h"

#include "engines/playground3d/gfx.h"

namespace Playground3d {

class N3DSRenderer : public Renderer {
public:
	N3DSRenderer(OSystem *_system);
	virtual ~N3DSRenderer();

	void init() override;
	void deinit() override;

	void setupCameraPerspective(float pitch, float heading, float fov) override;

	void clear(const Math::Vector4d &clearColor) override;
	void loadTextureRGBA(Graphics::Surface *texture) override;
	void loadTextureRGB(Graphics::Surface *texture) override;
	void loadTextureRGB565(Graphics::Surface *texture) override;
	void loadTextureRGBA5551(Graphics::Surface *texture) override;
	void loadTextureRGBA4444(Graphics::Surface *texture) override;

	void setupViewport(int x, int y, int width, int height) override;
	void drawCube(const Math::Vector3d &pos, const Math::Vector3d &roll) override;
	void drawPolyOffsetTest(const Math::Vector3d &pos, const Math::Vector3d &roll) override;
	void dimRegionInOut(float fade) override;
	void drawInViewport() override;
	void drawRgbaTexture() override;

	void enableFog(const Math::Vector4d &fogColor) override;
	void disableFog() override;

	void enableScissor(int x, int y, int width, int height) override;
	void disableScissor() override;

	void flipBuffer() override;

private:
	N3DS_3D::ContextHandle *_backendContext, *_p3dContext;
	C3D_Tex *_gameScreenTex;
	C3D_RenderTarget *_gameScreenTarget;
	C3D_TexEnv _p3dTexEnv;

	N3DS_3D::ShaderObj *_cubeShader;
	N3DS_3D::ShaderObj *_offsetShader;
	N3DS_3D::ShaderObj *_fadeShader;
	N3DS_3D::ShaderObj *_viewportShader;
	N3DS_3D::ShaderObj *_bitmapShader;

	void *_cubeVBO;
	void *_offsetVBO;
	void *_fadeVBO;
	void *_viewportVBO_1, *_viewportVBO_2;
	void *_bitmapVBO;

	Math::Vector3d _drawViewportPos;
	C3D_BufInfo _vpBuf2;

	Common::Rect _currentViewport;
	C3D_Tex _textureRgba;
	C3D_Tex _textureRgb;
	C3D_Tex _textureRgb565;
	C3D_Tex _textureRgba5551;
	C3D_Tex _textureRgba4444;
};

#undef SHADER_VARS
} // End of namespace Playground3d

#endif // PLAYGROUND3D_GFX_N3DS_H
