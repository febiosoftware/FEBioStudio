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
#include <cstdarg>

FSThreadedTask::FSThreadedTask()
{
	m_log = nullptr;
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

void FSThreadedTask::setErrorString(const std::string& s)
{
	m_error = s;
}

bool FSThreadedTask::error(const std::string& s)
{
	setErrorString(s);
	return false;
}

std::string FSThreadedTask::getErrorString() const
{
	return m_error;
}
