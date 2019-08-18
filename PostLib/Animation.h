#pragma once
class QImage;

//-----------------------------------------------------------------------------
//! Base class for creating animation
class CAnimation  
{
public:
	CAnimation();
	virtual ~CAnimation();

public:
	virtual int Create(const char* szfile, int cx, int cy, float fps = 10.f) = 0;
	virtual int Write(QImage& im) = 0;
	virtual bool IsValid() = 0;
	virtual void Close();
	virtual int Frames() = 0;
};
