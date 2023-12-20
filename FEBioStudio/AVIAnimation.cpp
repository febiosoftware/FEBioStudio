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
#ifdef WIN32
#include <Windows.h>
#endif
#include "AVIAnimation.h"
#include <QImage>
#ifdef WIN32

CAVIAnimation::CAVIAnimation()
{
	m_pfile = NULL;
	m_pavi = NULL;
	m_pavicmp = NULL;
	m_nsample = 0;

	m_buf = 0;
	m_bufSize = 0;
	m_bufLine = 0;
	m_cx = m_cy = 0;
}

void CAVIAnimation::Close()
{
	if (m_pavi) AVIStreamRelease(m_pavi); 
	if (m_pavicmp) AVIStreamRelease(m_pavi); 
	if (m_pfile) AVIFileRelease(m_pfile); 

	delete [] m_buf;
	m_buf = 0;
	m_bufSize = 0;
	m_bufLine = 0;

	m_pfile = 0;
	m_pavi = 0;
	m_pavicmp = 0;
	m_nsample = 0;
	m_cx = m_cy = 0;
}

int CAVIAnimation::Create(const char* szfile, int cx, int cy, float fps)
{
	// make sure the animation is closed
	Close();

	// fill the stream info
	ZeroMemory(&m_ai, sizeof(AVISTREAMINFO));
	m_ai.fccType = streamtypeVIDEO;			// type of stream (=VIDEO)
	m_ai.dwScale = 100;						// time scale factor (rate/scale = fps)
	m_ai.dwRate = (int)(m_ai.dwScale*fps);	// playback rate
	m_ai.rcFrame.left   = 0;
	m_ai.rcFrame.top    = 0;
	m_ai.rcFrame.right  = cx;
	m_ai.rcFrame.bottom = cy;
	m_ai.dwSuggestedBufferSize = 0; //cx*cy*3;

	// create a new avifile
	HRESULT hr = AVIFileOpenA(&m_pfile, szfile, OF_CREATE | OF_WRITE, NULL);
	if (hr != 0) return FALSE;

	// create a new video stream
	hr = AVIFileCreateStream(m_pfile, &m_pavi, &m_ai);
	if (hr != 0)
	{
		AVIFileRelease(m_pfile);
		m_pfile = 0;
		return FALSE;
	}

	// create a compressed video stream
	AVICOMPRESSOPTIONS ops = {0};
	AVICOMPRESSOPTIONS* pops = &ops;
	if (AVISaveOptions(NULL, 0, 1, &m_pavi, &pops))
	{
		hr = AVIMakeCompressedStream(&m_pavicmp, m_pavi, &ops, NULL);
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

	if (m_pavicmp == 0) return FALSE;

	// set the file format header info
	BITMAPINFO *pbmi = (BITMAPINFO*) &m_bmi;
	ZeroMemory(&pbmi->bmiHeader, sizeof(BITMAPINFOHEADER));	

	pbmi->bmiHeader.biSize		= sizeof(BITMAPINFOHEADER);
	pbmi->bmiHeader.biWidth		= cx;
	pbmi->bmiHeader.biHeight	= cy;
	pbmi->bmiHeader.biBitCount	= 24;
	pbmi->bmiHeader.biPlanes	= 1;

	// make sure that the bytes per line is a multiple of 4!
	int w = cx*3;
	if (w %4) w += 4 - w%4;

	m_bufLine = w;
	m_bufSize = cy*m_bufLine;
	m_buf = new unsigned char[m_bufSize];

	m_cx = cx;
	m_cy = cy;

	return TRUE;
}

int CAVIAnimation::Write(QImage& im)
{
	// make sure there is a file and a stream
	if ((m_pfile == NULL) || (m_pavi == NULL)) return FALSE;

	HRESULT hr;

	// if this is the first image set the AVI stream format
	if (m_nsample == 0)
	{
		hr = AVIStreamSetFormat(m_pavicmp, 0, (LPVOID) &m_bmi, sizeof(BITMAPINFOHEADER));
		if (hr != 0)
		{
			Close();
			return FALSE;
		}
	}
	
	// make sure the format is still the same
	if ((m_cx != im.width()) || (m_cy != im.height()))
	{
		Close();
		return FALSE;
	}

	// we need to flip the image and convert it to BGR
	int bytesPerLine = im.bytesPerLine();
	const uchar* s = im.bits();
	for (int y=0; y<m_cy; ++y)
	{
		const uchar* c = s + bytesPerLine*(m_cy - y - 1);
		unsigned char* d = m_buf + m_bufLine*y;
		for (int x=0; x<m_cx; ++x, d += 3, c += 4)
		{
			d[0] = c[0];
			d[1] = c[1];
			d[2] = c[2];
		}
	}

	// write the image data to the stream
	hr = AVIStreamWrite(m_pavicmp, m_nsample, 1, m_buf, m_bufSize, AVIIF_KEYFRAME, NULL, NULL);
	if (hr != 0)
	{
		// close the file
		Close();
		return FALSE;
	}

	m_nsample++;

	return TRUE;
}
#endif
