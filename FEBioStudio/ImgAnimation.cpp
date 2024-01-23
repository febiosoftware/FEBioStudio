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

#include "stdafx.h"
#include "ImgAnimation.h"
#include <assert.h>

CImgAnimation::CImgAnimation()
{
	Close();
}

int CImgAnimation::Create(const char* szfile, int cx, int cy, float fps)
{
	m_nx = cx;
	m_ny = cy;
	char sztmp[512] = {0};
	strcpy(sztmp, szfile);
	char* ch = strrchr(sztmp, '.');
	if (ch) *ch = 0;
	strcpy(m_szbase, sztmp);
	strcpy(m_szext, ch+1);
	return 1;
}

int CImgAnimation::Write(QImage& im)
{
	if (im.width() != m_nx) { assert(false); return 0; }
	if (im.height() != m_ny) { assert(false); return 0; }

	// create the next file name
	char szfile[512] = {0};
	sprintf(szfile, "%s%04d.%s", m_szbase, m_ncnt, m_szext);

	m_ncnt++;

	return (SaveFrame(im, szfile)? 1 : 0);
}

void CImgAnimation::Close()
{
	m_szbase[0] = 0;
	m_szext [0] = 0;
	m_bopen = false;
	m_ncnt = 0;
	m_nx = 0;
	m_ny = 0;
}

bool CImgAnimation::IsValid()
{
	return m_bopen;
}
