#pragma once
#include <vector>

//-------------------------------------------------------------------
// This class can be used to track a call stack. Macros assist
// with the use of this class.
class CCallStack
{
public:
	static void PushCall(const char* sz);
	static void PopCall();
	static void FlagError();
	static void ClearStack();

	static const std::vector<const char*> GetCallStack() { return m_stack; }

	static int GetCallStackString(char* sz);

private:
	CCallStack();

private:
	static std::vector<const char*> m_stack;
	static bool	m_blocked;	// lock stack
};

//-------------------------------------------------------------------
class CCallTracer
{
public:
	CCallTracer(const char* sz);
	~CCallTracer();
};

//-------------------------------------------------------------------
#define TRACE(s)	CCallTracer temp_tracer_obj(s);
