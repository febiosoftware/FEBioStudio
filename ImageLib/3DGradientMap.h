#pragma once
#include "3DImage.h"
#include <FSCore/box.h>

//-----------------------------------------------------------------------------
//! A class for calculating gradient data on a 3D image
class C3DGradientMap
{
public:
	C3DGradientMap(C3DImage& im, BOX box);
	~C3DGradientMap();

	// get a vector value
	vec3f Value(int i, int j, int k);

private:
	C3DImage&	m_im;
	BOX	m_box;
};

