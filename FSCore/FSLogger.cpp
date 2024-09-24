/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2023 University of Utah, The Trustees of Columbia University in
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
#include "FSLogger.h"
#include <cstdarg>
#include <assert.h>

FSLogOutput* FSLogger::m_log = nullptr;


void FSLogger::SetOutput(FSLogOutput* o) { m_log = o; }
void FSLogger::Write(const std::string& msg)
{
	if (m_log) m_log->Write(msg);
}

void FSLogger::Write(const char* sz, ...)
{
	if (m_log == nullptr) return;
	if ((sz == 0) || (*sz == 0)) return;

	// get a pointer to the argument list
	va_list	args;

	// copy to string
	char* szlog = NULL;

	va_start(args, sz);

	// count how many chars we need to allocate
	va_list argscopy;
	va_copy(argscopy, args);
	int l = vsnprintf(nullptr, 0, sz, argscopy) + 1;
	va_end(argscopy);
	if (l > 1)
	{
		szlog = new char[l]; assert(szlog);
		if (szlog)
		{
			vsnprintf(szlog, l, sz, args);
		}
	}
	va_end(args);
	if (szlog == NULL) return;

	m_log->Write(szlog);

	delete[] szlog;
}
