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
#include "AVIAnimation.h"
#ifdef WIN32
#include <Windows.h>
#include <vfw.h>
#include <QImage>

class CAVIAnimation::Imp 
{
public:
	PAVIFILE	pfile;	// the avifile pointer
	PAVISTREAM	pavi;		// the avistream pointer
	PAVISTREAM	pavicmp;	// the compressed avi stream
	AVISTREAMINFO* pai;

	int			nsample;	// index of next sample to write

	int			cx;		// width of animation rectangle
	int			cy;		// height of animation rectangle

	unsigned char* buf;		// pixel buffer
	int				bufSize;	// buffer size
	int				bufLine;	// bytes per line (must be multiple of 4!)

	AVISTREAMINFO		ai;
	BITMAPINFOHEADER	bmi;
};

CAVIAnimation::CAVIAnimation() : m(*(new Imp))
{
	m.pfile = NULL;
	m.pavi = NULL;
	m.pavicmp = NULL;
	m.nsample = 0;

	m.buf = 0;
	m.bufSize = 0;
	m.bufLine = 0;
	m.cx = m.cy = 0;
}

CAVIAnimation::~CAVIAnimation()
{
	delete& m;
}

void CAVIAnimation::Close()
{
	if (m.pavi) AVIStreamRelease(m.pavi); 
	if (m.pavicmp) AVIStreamRelease(m.pavi); 
	if (m.pfile) AVIFileRelease(m.pfile); 

	delete [] m.buf;
	m.buf = 0;
	m.bufSize = 0;
	m.bufLine = 0;

	m.pfile = 0;
	m.pavi = 0;
	m.pavicmp = 0;
	m.nsample = 0;
	m.cx = m.cy = 0;
}

bool CAVIAnimation::IsValid() { return (m.pfile != NULL); }
int CAVIAnimation::Frames() { return m.nsample; }

int CAVIAnimation::Create(const char* szfile, int cx, int cy, float fps)
{
	// make sure the animation is closed
	Close();

	// fill the stream info
	ZeroMemory(&m.ai, sizeof(AVISTREAMINFO));
	m.ai.fccType = streamtypeVIDEO;			// type of stream (=VIDEO)
	m.ai.dwScale = 100;						// time scale factor (rate/scale = fps)
	m.ai.dwRate = (int)(m.ai.dwScale*fps);	// playback rate
	m.ai.rcFrame.left   = 0;
	m.ai.rcFrame.top    = 0;
	m.ai.rcFrame.right  = cx;
	m.ai.rcFrame.bottom = cy;
	m.ai.dwSuggestedBufferSize = 0; //cx*cy*3;

	// create a new avifile
	HRESULT hr = AVIFileOpenA(&m.pfile, szfile, OF_CREATE | OF_WRITE, NULL);
	if (hr != 0) return FALSE;

	// create a new video stream
	hr = AVIFileCreateStream(m.pfile, &m.pavi, &m.ai);
	if (hr != 0)
	{
		AVIFileRelease(m.pfile);
		m.pfile = 0;
		return FALSE;
	}

	// create a compressed video stream
	AVICOMPRESSOPTIONS ops = {0};
	AVICOMPRESSOPTIONS* pops = &ops;
	if (AVISaveOptions(NULL, 0, 1, &m.pavi, &pops))
	{
		hr = AVIMakeCompressedStream(&m.pavicmp, m.pavi, &ops, NULL);
		if (hr != 0)
		{
			Close();
			return FALSE;
		}
	}
	else
	{
		Close();
		return FALSE;
	}

	if (m.pavicmp == 0) return FALSE;

	// set the file format header info
	BITMAPINFO *pbmi = (BITMAPINFO*) &m.bmi;
	ZeroMemory(&pbmi->bmiHeader, sizeof(BITMAPINFOHEADER));	

	pbmi->bmiHeader.biSize		= sizeof(BITMAPINFOHEADER);
	pbmi->bmiHeader.biWidth		= cx;
	pbmi->bmiHeader.biHeight	= cy;
	pbmi->bmiHeader.biBitCount	= 24;
	pbmi->bmiHeader.biPlanes	= 1;

	// make sure that the bytes per line is a multiple of 4!
	int w = cx*3;
	if (w %4) w += 4 - w%4;

	m.bufLine = w;
	m.bufSize = cy*m.bufLine;
	m.buf = new unsigned char[m.bufSize];

	m.cx = cx;
	m.cy = cy;

	return TRUE;
}

int CAVIAnimation::Write(QImage& im)
{
	// make sure there is a file and a stream
	if ((m.pfile == NULL) || (m.pavi == NULL)) return FALSE;

	HRESULT hr;

	// if this is the first image set the AVI stream format
	if (m.nsample == 0)
	{
		hr = AVIStreamSetFormat(m.pavicmp, 0, (LPVOID) &m.bmi, sizeof(BITMAPINFOHEADER));
		if (hr != 0)
		{
			Close();
			return FALSE;
		}
	}
	
	// make sure the format is still the same
	if ((m.cx != im.width()) || (m.cy != im.height()))
	{
		Close();
		return FALSE;
	}

	// we need to flip the image and convert it to BGR
	int bytesPerLine = im.bytesPerLine();
	const uchar* s = im.bits();
	for (int y=0; y<m.cy; ++y)
	{
		const uchar* c = s + bytesPerLine*(m.cy - y - 1);
		unsigned char* d = m.buf + m.bufLine*y;
		for (int x=0; x<m.cx; ++x, d += 3, c += 4)
		{
			d[0] = c[0];
			d[1] = c[1];
			d[2] = c[2];
		}
	}

	// write the image data to the stream
	hr = AVIStreamWrite(m.pavicmp, m.nsample, 1, m.buf, m.bufSize, AVIIF_KEYFRAME, NULL, NULL);
	if (hr != 0)
	{
		// close the file
		Close();
		return FALSE;
	}

	m.nsample++;

	return TRUE;
}
#endif
