#include "ImageSlice.h"
#include <assert.h>
#include <math.h>

// find the power of 2 that is closest to n
int closest_pow2(int n)
{
	const int PMIN = 16;
	const int PMAX = 512;
	int p = (int)pow(2.0, (int)(0.5 + log((double)n) / log(2.0)));
	if (p < PMIN) p = PMIN;
	if (p > PMAX) p = PMAX;
	return p;
}


ImageSlice::ImageSlice()
{
	m_op = 0;
	m_off = 0.5;
}

ImageSlice::~ImageSlice()
{
}

void ImageSlice::Create(ImageStack& im3d)
{
	// copy image data
	m_im3d = im3d;

	// call update to initialize all other data
	Update();
}

void ImageSlice::Update()
{
	// get the 2D image
	switch (m_op)
	{
	case 0: // X
		m_im3d.GetSampledSliceX(m_im, m_off);
		break;
	case 1: // Y
		m_im3d.GetSampledSliceY(m_im, m_off);
		break;
	case 2: // Z
		m_im3d.GetSampledSliceZ(m_im, m_off);
		break;
	default:
		assert(false);
	}
}
