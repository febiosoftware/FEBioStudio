/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#include "FEDataMap.h"

FEDataMapGenerator::FEDataMapGenerator()
{
	m_type = 0;
}

//======================================================================
FESurfaceToSurfaceMap::FESurfaceToSurfaceMap()
{
	m_type = ELEM_DATA_GENERATOR;

	AddStringParam("", "bottom_surface", "Bottom surface");
	AddStringParam("", "top_surface"   , "Top surface");
	AddDoubleParam(1.0, "function");
}

void FESurfaceToSurfaceMap::SetBottomSurface(const std::string& surfName) { SetStringValue(0, surfName); }
void FESurfaceToSurfaceMap::SetTopSurface(const std::string& surfName) { SetStringValue(1, surfName); }

std::string FESurfaceToSurfaceMap::GetBottomSurface() const { return GetStringValue(0); }
std::string FESurfaceToSurfaceMap::GetTopSurface() const { return GetStringValue(1); }

//======================================================================
FESurfaceConstVec3d::FESurfaceConstVec3d()
{
	m_type = FACE_DATA_GENERATOR;
	AddVecParam(vec3d(0, 0, 0), "value");

	m_generator = "const";
}

vec3d FESurfaceConstVec3d::Value() const
{
	return GetVecValue(0);
}

void FESurfaceConstVec3d::SetValue(const vec3d& v)
{
	SetVecValue(0, v);
}
