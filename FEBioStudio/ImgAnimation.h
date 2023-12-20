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
#include "Animation.h"
#include <QImage>

class CImgAnimation : public CAnimation
{
protected:
	CImgAnimation();

public:
	int Create(const char* szfile, int cx, int cy, float fps = 10.f) override;
	int Write(QImage& im) override;
	void Close() override;
	bool IsValid() override;
	int Frames() override { return m_ncnt; }

	virtual bool SaveFrame(QImage& im, const char* szfile) = 0;

protected:
	int	m_ncnt;	
	int	m_nx;
	int	m_ny;
	bool	m_bopen;	//!< is the "file" open or not

	char	m_szbase[512];	//!< base for generating file names
	char	m_szext [512];	//!< extension for generating file names
};


class CBmpAnimation : public CImgAnimation
{
public:
	CBmpAnimation(){}
	virtual bool SaveFrame(QImage& im, const char* szfile) { return im.save(szfile, "BMP"); };
};

class CPNGAnimation : public CImgAnimation
{
public:
	CPNGAnimation(){}
	virtual bool SaveFrame(QImage& im, const char* szfile) { return im.save(szfile, "PNG"); };
};

class CJpgAnimation : public CImgAnimation
{
public:
	CJpgAnimation(){}
	virtual bool SaveFrame(QImage& im, const char* szfile) { return im.save(szfile, "JPG"); };
};
