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
#include "FunctionTimer.h"
#include "FSLogger.h"
#include <chrono>
#include <assert.h>
using namespace std::chrono;
using dseconds = duration<double>;

int FunctionTimer::l = 0;

static char sz[] = "--------------";

struct FunctionTimer::Imp 
{
	const char* szname = nullptr;
	time_point<steady_clock>	tic;
};

FunctionTimer::~FunctionTimer()
{
	time_point<steady_clock> toc = steady_clock::now();
	double secs = duration_cast<dseconds>(toc - m.tic).count();
	const char* szname = (m.szname ? m.szname : "<unknown>");
	l--; assert(l >= 0);
	sz[l] = 0;

	FSLogger::Write("%s%s : %lg secs.\n", sz, m.szname, secs);
	delete& m;
}

FunctionTimer::FunctionTimer(const char* szname) : m(*(new FunctionTimer::Imp))
{
	m.szname = szname;
	m.tic = steady_clock::now();
	sz[l] = '-'; sz[l + 1] = 0;
	l++;
}
