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

/*
 * This file is based on, or a modified version of code from TinyGL (C) 1997-2022 Fabrice Bellard,
 * which is licensed under the MIT license (see LICENSE).
 * It also has modifications by the ResidualVM-team, which are covered under the GPLv2 (or later).
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


#include "graphics/3ds/ngl.h"
//#include "graphics/3ds/z3d.h"

#define N3DCONTEXT_FROM_HANDLE(handle) ((N3DS_3D::N3DContext *)handle)

namespace N3DS_3D {

typedef void *ContextHandle;

ContextHandle *createContext(ContextHandle *source = nullptr);
ContextHandle *createOGLContext();
void destroyNative3D();
void destroyContext(ContextHandle *handle);
void setContext(ContextHandle *handle);


} // end of namespace N3DS_3D

#endif // GRAPHICS_N3DS_NATIVE3D_H
