/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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
#include <cstddef>

// functions for swapping data (used by some binary file import/export classes)
void inline bswap(short& s)
{
	unsigned char* c = (unsigned char*)(&s);
	c[0] ^= c[1]; c[1] ^= c[0]; c[0] ^= c[1];
}

void inline bswap(int& n)
{
	unsigned char* c = (unsigned char*)(&n);
	c[0] ^= c[3]; c[3] ^= c[0]; c[0] ^= c[3];
	c[1] ^= c[2]; c[2] ^= c[1]; c[1] ^= c[2];
}

void inline bswap(unsigned int& n)
{
	unsigned char* c = (unsigned char*)(&n);
	c[0] ^= c[3]; c[3] ^= c[0]; c[0] ^= c[3];
	c[1] ^= c[2]; c[2] ^= c[1]; c[1] ^= c[2];
}

void inline bswap(float& f)
{
	unsigned char* c = (unsigned char*)(&f);
	c[0] ^= c[3]; c[3] ^= c[0]; c[0] ^= c[3];
	c[1] ^= c[2]; c[2] ^= c[1]; c[1] ^= c[2];
}

void inline bswap(double& g)
{
	unsigned char* c = (unsigned char*)(&g);
	c[0] ^= c[7]; c[7] ^= c[0]; c[0] ^= c[7];
	c[1] ^= c[6]; c[6] ^= c[1]; c[1] ^= c[6];
	c[2] ^= c[5]; c[5] ^= c[2]; c[2] ^= c[5];
	c[3] ^= c[4]; c[4] ^= c[3]; c[3] ^= c[4];
}

template <typename T> void bswapv(T* pd, int n)
{
	for (int i = 0; i<n; ++i) bswap(pd[i]);
}

// helper function for reading from a memory buffer
void mread(void* pdest, size_t Size, size_t Cnt, void** psrc);
