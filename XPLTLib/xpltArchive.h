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
#include <stdio.h>
#include <string.h>
#include <vector>
#include <FSCore/math3d.h>
#include <FSCore/Archive.h>

//-----------------------------------------------------------------------------
// Input archive
class xpltArchive  
{
	class Imp;

	struct CHUNK
	{
		unsigned int	id;		// chunk ID
		unsigned int	nsize;	// size of chunk
		void*			pdata;	// pointer to data
	};

public:
	enum IOResult { IO_ERROR, IO_OK, IO_END };

public:
	//! class constructor
	xpltArchive();

	//! destructor
	virtual ~xpltArchive();

public:
	// --- Writing ---

	// Open for writing
	bool Create(const char* szfile);

	// begin a chunk
	void BeginChunk(unsigned int id);

	// end a chunck
	void EndChunk();

	template <typename T> void WriteChunk(unsigned int nid, T& o)
	{
		AddChild(new OLeaf<T>(nid, o));
	}

	void WriteChunk(unsigned int nid, const char* sz)
	{
		AddChild(new OLeaf<const char*>(nid, sz));
	}

	template <typename T> void WriteChunk(unsigned int nid, T* po, int n)
	{
		AddChild(new OLeaf<T*>(nid, po, n));
	}

	template <typename T> void WriteChunk(unsigned int nid, std::vector<T>& a)
	{
		AddChild(new OLeaf<std::vector<T> >(nid, a));
	}

	// (overridden from Archive)
	virtual void WriteData(int nid, std::vector<float>& data)
	{
		WriteChunk(nid, data);
	}

protected:
	void AddChild(OChunk* c);

public: // reading 

	// Close the archive
	void Close();

	void Flush();

	// Open for reading
	bool Open(FileStream* fp);

	// open for appending
	bool Append(const char* szfile);

	// Open a chunk
	int OpenChunk();

	// Get the current chunk ID
	unsigned int GetChunkID();

	// Get the current chunk size
	unsigned int GetChunkSize();

	// Close a chunk
	void CloseChunk();

	// input functions
	IOResult read(char&   c);
	IOResult read(int&    n);
	IOResult read(bool&   b);
	IOResult read(float&  f);
	IOResult read(double& g);

	IOResult read(unsigned int& n);


	IOResult read(char*   pc, int n);
	IOResult read(int*    pi, int n);
	IOResult read(bool*   pb, int n);
	IOResult read(float*  pf, int n);
	IOResult read(double* pg, int n);

	IOResult read(char* sz);

	IOResult sread(char* sz, int max_len);

	IOResult read(vec3f&   a) { return read(&(a.x), 3); }
	IOResult read(mat3fs&  a) { return read(&(a.x), 6); }
	IOResult read(mat3fd&  a) { return read(&(a.x), 3); }
	IOResult read(tens4fs& a) { return read(&(a.d[0]), 21); }
	IOResult read(mat3f&   a) { return read(&(a.d[0][0]), 9); }

	IOResult read(std::vector<int    >& a) { return read(&a[0], (int) a.size()); }
	IOResult read(std::vector<float  >& a) { return read(&a[0], (int) a.size()); }
	IOResult read(std::vector<vec3f  >& a) { return read(&(a[0].x), 3*(int) a.size()); }
	IOResult read(std::vector<mat3fs >& a) { return read(&(a[0].x), 6*(int) a.size()); }
	IOResult read(std::vector<mat3fd >& a) { return read(&(a[0].x), 3*(int) a.size()); }
	IOResult read(std::vector<tens4fs>& a) { return read(&(a[0].d[0]), 21*(int) a.size()); }
	IOResult read(std::vector<mat3f  >& a) { return read(&(a[0].d[0][0]), 9*(int) a.size()); }
	IOResult read(std::vector<unsigned int>& a) { return read((int*)&a[0], (int)a.size()); }

	void SetVersion(unsigned int n);
	unsigned int Version();

	// set/get compression method
	int GetCompression();
	void SetCompression(int n);

	bool DecompressChunk(unsigned int& nid, unsigned int& nsize);

protected:
	Imp& im;
};
