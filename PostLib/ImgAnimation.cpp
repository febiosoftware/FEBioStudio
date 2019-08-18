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
