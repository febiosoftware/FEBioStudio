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
#pragma once
#include <ImageLib/3DImage.h>
#include <FSCore/color.h>

class GLTexture3D
{
public:
	GLTexture3D();

	unsigned int GetTexID() const { return m_texID; }
	void SetTexID(unsigned int id) { m_texID = id; m_isModified = true; }

	void Set3DImage(C3DImage* im) { m_im = im; m_isModified = true; }
	C3DImage* Get3DImage() { return m_im; }

	bool IsModified() const { return m_isModified; }
	void SetModified(bool b = true) { m_isModified = b; }

public:
	float Imin;
	float Imax;
	float Amin;
	float Amax;
	float gamma;
	float hue;
	float sat;
	float val;
	int cmap;
	float IscaleMin;
	float Iscale;

	GLColor col1, col2, col3;

private:
	unsigned int m_texID;
	C3DImage* m_im;
	bool	m_isModified;
};
