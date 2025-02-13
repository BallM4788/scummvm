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

//#include "graphics/opengl/shader.h"
//#include "graphics/opengl/system_headers.h"

#include "graphics/3ds/n3d.h"

#include "engines/playground3d/gfx.h"

namespace Playground3d {

//typedef struct {
//	float position[2];
//} Vtx;
//
//typedef struct {
//	float position[2];
//	float texcoord[2];
//} VtxTex;

class N3DSRenderer : public Renderer {
public:
	N3DSRenderer(OSystem *_system);
	virtual ~N3DSRenderer();

	void init() override;
	void deinit() override;

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

	void flipBuffer() override;

private:
	N3DS_3D::ContextHandle *_backendContext, *_p3dContext;
	C3D_Tex *_gameScreenTex;
	C3D_RenderTarget *_gameScreenTarget;
	C3D_TexEnv _p3dTexEnv;

//	OpenGL::Shader *_cubeShader;
//	OpenGL::Shader *_fadeShader;
//	OpenGL::Shader *_bitmapShader;
	shaderProgram_s *_cubeProgram;
	shaderProgram_s *_offsetProgram;
	shaderProgram_s *_fadeProgram;
	u8 _cubeProgramFlags;
	u8 _offsetProgramFlags;
	u8 _fadeProgramFlags;
	N3DS_3D::ShaderObj *_cubeShader;
	N3DS_3D::ShaderObj *_offsetShader;
	N3DS_3D::ShaderObj *_fadeShader;
	// N3DS_3D::ShaderObj *_bitmapShader;

//	GLuint _cubeVBO;
//	GLuint _fadeVBO;
//	GLuint _bitmapVBO;
	void *_cubeVBO;
	void *_offsetVBO;
	void *_fadeVBO;
	// void *_bitmapVBO;

	Common::Rect _currentViewport;
//	GLuint _textureRgbaId[5];
//	GLuint _textureRgbId[5];
//	GLuint _textureRgb565Id[2];
//	GLuint _textureRgba5551Id[2];
//	GLuint _textureRgba4444Id[2];
	// C3D_Tex _textureRgbaId[5];
	// C3D_Tex _textureRgbId[5];
	// C3D_Tex _textureRgb565Id[2];
	// C3D_Tex _textureRgba5551Id[2];
	// C3D_Tex _textureRgba4444Id[2];
};

} // End of namespace Playground3d

#endif // PLAYGROUND3D_GFX_OPENGL_SHADERS_H
