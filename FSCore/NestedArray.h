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
#include <vector>

template <class T>
struct NestedArray
{
	std::vector<int> ind; // size n + 1
	std::vector<T> data;

	int items(int n) const { return ind[n + 1] - ind[n]; }
	T& item(int n, int j) {
		assert(j < items(n));
		return data[ind[n] + j];
	}

	const T& item(int n, int j) const {
		assert(j < items(n));
		return data[ind[n] + j];
	}

	void clear()
	{
		ind.clear();
		data.clear();
	}

	bool empty() const { return data.empty(); }

	void resize(std::vector<int>& val)
	{
		clear();

		size_t N = val.size();
		ind.resize(N + 1);
		int n0 = 0;
		for (int i = 0; i < N; ++i)
		{
			int ni = val[i];
			ind[i] = n0;
			n0 += ni;
			val[i] = 0;
		}
		ind[N] = n0;
		data.resize(n0);
	}
};
