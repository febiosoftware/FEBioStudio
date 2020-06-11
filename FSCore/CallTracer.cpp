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

#include "stdafx.h"
#include "CallTracer.h"
#include <assert.h>
#include <cstring>

//-------------------------------------------------------------------
std::vector<const char*> CCallStack::m_stack;
bool CCallStack::m_blocked = false;

//-------------------------------------------------------------------
void CCallStack::PushCall(const char* sz)
{
	m_blocked = false;
	m_stack.push_back(sz);
}

//-------------------------------------------------------------------
void CCallStack::PopCall()
{
	if (m_blocked == false) m_stack.pop_back();
}

//-------------------------------------------------------------------
void CCallStack::FlagError()
{
	m_blocked = true;
}

//-------------------------------------------------------------------
void CCallStack::ClearStack()
{
	m_stack.clear();
}

//-------------------------------------------------------------------
int CCallStack::GetCallStackString(char* sz)
{
	int L = 0;
	int N = (int)m_stack.size();
	for (int i=0; i<N; ++i)
	{
		const char* szi = m_stack[i];
		L += (int)strlen(szi) + 1;
	}
	L -= 1;
	if (sz == 0) return L;

	sz[0] = 0;
	for (int i=0; i<N; ++i)
	{
		const char* szi = m_stack[i];
		strcat(sz, szi);
		if (i != N-1)  strcat(sz, "\n");
	}
	assert(L == strlen(sz));
	return L;
}

//-------------------------------------------------------------------
CCallTracer::CCallTracer(const char* sz)
{
	CCallStack::PushCall(sz);
}

//-------------------------------------------------------------------
CCallTracer::~CCallTracer()
{
	CCallStack::PopCall();
}
