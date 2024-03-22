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
#include "stdafx.h"
#include "FSThreadedTask.h"
#include <stdarg.h>
#include <cstdarg>

FSThreadedTask::FSThreadedTask()
{
	m_nerrors = 0;
    m_log = nullptr;
}
	
void FSThreadedTask::Reset()
{
	setProgress(0);
	ClearErrors();
}

FSTaskProgress FSThreadedTask::GetProgress()
{
	return m_progress;
}

void FSThreadedTask::Terminate()
{
	m_progress.valid = false;
	m_progress.canceled = true;
}

void FSThreadedTask::SetTaskLogger(TaskLogger* logger)
{
	m_log = logger;
}

void FSThreadedTask::Log(const char* sz, ...)
{
	if (m_log == nullptr) return;

	if ((sz == 0) || (*sz == 0)) return;

	// get a pointer to the argument list
	va_list	args, copy;

	// copy to string
	char* szlog = NULL;

	va_start(args, sz);
    
    va_copy(copy, args);
	// count how many chars we need to allocate
	int l = vsnprintf(nullptr, 0, sz, copy) + 1;
    va_end(copy);

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

	m_log->Log(szlog);
	delete [] szlog;
}

bool FSThreadedTask::IsCanceled() const
{
	return m_progress.canceled;
}

void FSThreadedTask::setProgress(double progress)
{
	if (progress < 0.0) progress = 0.0;
	if (progress > 100.0) progress = 100.0;
	m_progress.valid = true;
	m_progress.percent = progress;
}

void FSThreadedTask::resetProgress()
{
	m_progress.valid = false;
	m_progress.percent = 0;
	m_progress.canceled = false;
	m_progress.task = nullptr;
}

void FSThreadedTask::setCurrentTask(const char* sz, double progress)
{
	setProgress(progress);
	m_progress.task = sz;
	Log("%s\n", sz);
}

void FSThreadedTask::ClearErrors()
{
	m_err.clear();
	m_nerrors = 0;
}

void FSThreadedTask::setErrorString(const std::string& s)
{
	m_err = s;
}

bool FSThreadedTask::errf(const char* szerr, ...)
{
	if (szerr == 0) return false;

	// get a pointer to the argument list
	va_list	args;

	// copy to string
	va_start(args, szerr);

	int l = strlen(szerr) + 1024;
	char* sz = new char[l + 1];
#ifdef WIN32
	vsprintf_s(sz, l, szerr, args);
#else
	vsnprintf(sz, l, szerr, args);
#endif
	sz[l] = 0;
	va_end(args);

	// append to the error string
	if (m_err.empty())
	{
		m_err = string(sz);
	}
	else m_err.append("\n").append(sz);

	delete[] sz;

	m_nerrors++;

	return false;
}

bool FSThreadedTask::error(const std::string& err)
{
	if (err.empty() == false)
	{
		if (m_err.empty()) m_err = err;
		else m_err.append("\n").append(err);
	}
	return false;
}

std::string FSThreadedTask::GetErrorString() const
{
	return m_err;
}

int FSThreadedTask::Errors() const
{
	return m_nerrors;
}
