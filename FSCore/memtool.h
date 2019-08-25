#pragma once

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
