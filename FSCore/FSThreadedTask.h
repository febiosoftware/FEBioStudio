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
#include "FSObject.h"

struct FSTaskProgress
{
	bool		valid;
	double		percent;
	const char*	task;
	bool		canceled;

	FSTaskProgress()
	{
		valid = false;
		canceled = false;
		percent = 0.0;
		task = "";
	}

	FSTaskProgress(double p, const char* sz)
	{
		valid = false;
		canceled = false;
		percent = p;
		task = sz;
	}
};

class TaskLogger
{
public:
	TaskLogger() {}
	virtual ~TaskLogger() {}
	virtual void Log(const std::string& msg) = 0;
};

class FSThreadedTask : public FSObject
{
public:
	FSThreadedTask();

	// return progress
	virtual FSTaskProgress GetProgress();

	// The thread is about to be terminated
	virtual void Terminate();

	// see if the task was canceled? 
	virtual bool IsCanceled() const;

	std::string getErrorString() const;

	void SetTaskLogger(TaskLogger* logger);

	void Log(const char* sz, ...);

protected:
	// reset progress 
	void resetProgress();

	// set progress in percent (value between 0 and 100)
	void setProgress(double d);

	// set task, and optionally, set progress in percent (value between 0 and 100)
	void setCurrentTask(const char* sz, double progress = 0.0);

	void setErrorString(const std::string& s);

	bool error(const std::string& s);

private:
	FSTaskProgress m_progress;
	std::string	m_error;
	TaskLogger* m_log;
};
