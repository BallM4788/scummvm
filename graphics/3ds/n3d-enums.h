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

#ifndef GRAPHICS_3DS_3D_ENUMS_H
#define GRAPHICS_3DS_3D_ENUMS_H

enum N3D_CULLFACE {
	N3D_CULLFACE_FRONT          = 0,
	N3D_CULLFACE_BACK           = 1,
	N3D_CULLFACE_FRONT_AND_BACK = 2,
};

enum N3D_FRONTFACE {
	N3D_FRONTFACE_CW  = 0,
	N3D_FRONTFACE_CCW = 1,
};

#endif
