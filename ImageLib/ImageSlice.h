#pragma once
#include "ImageStack.h"

class ImageSlice
{
public:
	ImageSlice();
	~ImageSlice();

	void Create(ImageStack& im);

	int GetOrientation() const { return m_op; }
	void SetOrientation(int n) { m_op = n; }

	double GetOffset() const { return m_off; }
	void SetOffset(double f) { m_off = f; }

	void Update();

private:
	ImageStack		m_im3d;				// 3D image data
	Image			m_im;				// 2D image that will be displayed
	int				m_LUTC[4][256];		// color lookup table
	int				m_op;				// x,y,z
	double			m_off;				// offset (0 - 1)
};
