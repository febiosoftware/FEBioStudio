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
#include "RTMath.h"
#include <ImageLib/3DImage.h>
#include <ImageLib/RGBAImage.h>

namespace rt {

	class Texture1D
	{
	public:
		Texture1D();
		~Texture1D();

		void setImageData(size_t n, unsigned char* bytes);

		rt::Color sample(float r);

	private:
		size_t size = 0;
		float* data = nullptr;
	};

	class Texture2D
	{
	public:
		Texture2D();
		~Texture2D();

		bool isNull() const { return img.isNull(); }

		void setImageData(const CRGBAImage& image);

		rt::Color sample(float r, float s);

	private:
		CRGBAImage img;
	};

	class Texture3D
	{
	public:
		Texture3D();
		~Texture3D();

		void setImageData(C3DImage* img3d);

		rt::Color sample(float r, float s, float t);

	private:
		C3DImage* img;
	};
}
