#pragma once
#include <PostLib/Animation.h>
#ifdef WIN32
#include <vfw.h>

//-----------------------------------------------------------------------------
//! Base class for creating animation
class CAVIAnimation : public CAnimation
{
public:
	CAVIAnimation();

public:
	int Create(const char* szAVIFile, int cx, int cy, float fps = 10.f);
	int Write(QImage& im);
	void Close();
	bool IsValid() { return (m_pfile != NULL); }
	int Frames() override { return m_nsample; }

protected:
	PAVIFILE	m_pfile;	// the avifile pointer
	PAVISTREAM	m_pavi;		// the avistream pointer
	PAVISTREAM	m_pavicmp;	// the compressed avi stream
	AVISTREAMINFO* m_pai;

	int			m_nsample;	// index of next sample to write

	int			m_cx;		// width of animation rectangle
	int			m_cy;		// height of animation rectangle

	unsigned char*	m_buf;		// pixel buffer
	int				m_bufSize;	// buffer size
	int				m_bufLine;	// bytes per line (must be multiple of 4!)

	AVISTREAMINFO		m_ai;
	BITMAPINFOHEADER	m_bmi;
};
#endif
