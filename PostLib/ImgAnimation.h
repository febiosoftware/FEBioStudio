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
