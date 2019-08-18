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
